//
// C++ Implementation: StatusIcon
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "StatusIcon.h"

#include "DataSettings.h"
#include "DetectPal.h"
#include "DialogGroup.h"
#include "DialogPeer.h"
#include "ShareFile.h"
#include "callback.h"
#include "global.h"
#include "iptux/deplib.h"
#include "support.h"
#include "utils.h"

namespace iptux {

/**
 * 类构造函数.
 */
StatusIcon::StatusIcon(IptuxConfig &config, MainWindow &mwin)
    : config(config), mwin(mwin), statusicon(NULL), timerid(0) {}

/**
 * 类析构函数.
 */
StatusIcon::~StatusIcon() {
  if (statusicon) g_object_unref(statusicon);
  if (timerid > 0) g_source_remove(timerid);
}

/**
 * 创建状态图标.
 */
void StatusIcon::CreateStatusIcon() {
  GdkScreen *screen;

  if (g_progdt->IsAutoHidePanelAfterLogin()) {
    statusicon = gtk_status_icon_new_from_stock("iptux-logo-hide");
    g_object_set_data(G_OBJECT(statusicon), "show", GINT_TO_POINTER(FALSE));
  } else {
    statusicon = gtk_status_icon_new_from_stock("iptux-logo-show");
    g_object_set_data(G_OBJECT(statusicon), "show", GINT_TO_POINTER(TRUE));
  }
  screen = gdk_screen_get_default();
  gtk_status_icon_set_screen(statusicon, screen);

  g_signal_connect_swapped(statusicon, "activate",
                           G_CALLBACK(StatusIconActivate), this);
  g_signal_connect_swapped(statusicon, "popup-menu", G_CALLBACK(onPopupMenu),
                           this);
  g_object_set(statusicon, "has-tooltip", TRUE, NULL);
  g_signal_connect(statusicon, "query-tooltip",
                   G_CALLBACK(StatusIconQueryTooltip), NULL);
}

/**
 * 更改状态图标的表现形式.
 */
void StatusIcon::AlterStatusIconMode() {
  if (g_object_get_data(G_OBJECT(statusicon), "show")) {
    gtk_status_icon_set_from_stock(statusicon, "iptux-logo-hide");
    g_object_set_data(G_OBJECT(statusicon), "show", GINT_TO_POINTER(FALSE));
  } else {
    gtk_status_icon_set_from_stock(statusicon, "iptux-logo-show");
    g_object_set_data(G_OBJECT(statusicon), "show", GINT_TO_POINTER(TRUE));
  }
}

/**
 * 创建弹出菜单.
 * @return 菜单
 */
GtkWidget *StatusIcon::CreatePopupMenu() {
  GtkWidget *menu, *menuitem;
  GtkWidget *image, *window;

  window = mwin.ObtainWindow();
  menu = gtk_menu_new();

  /* 显示&隐藏面板 */
  if (g_object_get_data(G_OBJECT(statusicon), "show"))
    menuitem = gtk_image_menu_item_new_with_mnemonic(_("_Hide"));
  else
    menuitem = gtk_image_menu_item_new_with_mnemonic(_("_Show"));
  image = gtk_image_new_from_icon_name("menu-board", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
  g_signal_connect_swapped(menuitem, "activate", G_CALLBACK(onActivate), this);

  menuitem = gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

  /* 显示文件传输窗口 */
  NO_OPERATION_C
  menuitem = gtk_image_menu_item_new_with_mnemonic(_("_Transmission"));
  image = gtk_image_new_from_stock(GTK_STOCK_CONNECT, GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
  g_signal_connect_swapped(menuitem, "activate", G_CALLBACK(ShowTransWindow),
                           this);

  /* 首选项 */
  NO_OPERATION_C
  menuitem = gtk_image_menu_item_new_with_mnemonic(_("_Preferences"));
  image = gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
  g_signal_connect_swapped(menuitem, "activate",
                           G_CALLBACK(DataSettings::ResetDataEntry), window);

  /* 共享文件管理 */
  NO_OPERATION_C
  menuitem = gtk_image_menu_item_new_with_mnemonic(_("_Shared Management"));
  image = gtk_image_new_from_icon_name("menu-share", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
  g_signal_connect_swapped(menuitem, "activate",
                           G_CALLBACK(ShareFile::ShareEntry), window);

  menuitem = gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

  /* 探测好友 */
  NO_OPERATION_C
  menuitem = gtk_image_menu_item_new_with_mnemonic(_("_Detect"));
  image = gtk_image_new_from_icon_name("menu-detect", GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
  g_signal_connect_swapped(menuitem, "activate",
                           G_CALLBACK(DetectPal::DetectEntry), window);

  /* 程序退出 */
  NO_OPERATION_C
  menuitem = gtk_image_menu_item_new_with_mnemonic(_("_Quit"));
  image = gtk_image_new_from_stock(GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
  g_signal_connect(menuitem, "activate", G_CALLBACK(iptux_gui_quit), NULL);

  return menu;
}

/**
 * 显示文件传输窗口.
 */
void StatusIcon::ShowTransWindow(StatusIcon *self) {
  self->mwin.OpenTransWindow();
}

/**
 * 状态图标被激活的响应处理函数.
 */
void StatusIcon::StatusIconActivate(StatusIcon *self) {
  GroupInfo *grpinf;

  g_cthrd->Lock();
  if (g_cthrd->GetMsglineItems())
    grpinf = g_cthrd->GetMsglineHeadItem();
  else
    grpinf = NULL;
  g_cthrd->Unlock();
  if (grpinf) {
    switch (grpinf->type) {
      case GROUP_BELONG_TYPE_REGULAR:
        DialogPeer::PeerDialogEntry(self->config, grpinf,
                                    self->getProgramData());
        break;
      case GROUP_BELONG_TYPE_SEGMENT:
      case GROUP_BELONG_TYPE_GROUP:
      case GROUP_BELONG_TYPE_BROADCAST:
        DialogGroup::GroupDialogEntry(self->config, grpinf,
                                      self->getProgramData());
        break;
      default:
        break;
    }
  } else {
    onActivate(self);
  }
}

/**
 * 弹出菜单.
 * @param statusicon the object which received the signal
 * @param button the button that was pressed
 * @param time the timestamp of the event that triggered the signal emission
 */
void StatusIcon::onPopupMenu(StatusIcon *self, guint button, guint time) {
  GtkWidget *menu;

  menu = self->CreatePopupMenu();
  gtk_widget_show_all(menu);
  gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, button, time);
}

/**
 * 状态图标信息提示查询请求.
 * @param statusicon the object which received the signal
 * @param x the x coordinate of the cursor position
 * @param y the y coordinate of the cursor position
 * @param key TRUE if the tooltip was trigged using the keyboard
 * @param tooltip a GtkTooltip
 * @return Gtk+库所需
 */
gboolean StatusIcon::StatusIconQueryTooltip(GtkStatusIcon *statusicon, gint x,
                                            gint y, gboolean key,
                                            GtkTooltip *tooltip) {
  char *msgstr;
  guint len;

  /* 获取消息串 */
  g_cthrd->Lock();
  if ((len = g_cthrd->GetMsglineItems())) {
    msgstr = g_strdup_printf(_("To be read: %u messages"), len);
  } else {
    msgstr = get_sys_host_addr_string(g_cthrd->getUdpSock());
    msgstr = msgstr ? msgstr : g_strdup(_("iptux"));
  }
  g_cthrd->Unlock();
  /* 设置信息提示串 */
  gtk_tooltip_set_text(tooltip, msgstr);
  g_free(msgstr);

  return TRUE;
}

/**
 * 状态图标是否嵌入到状态栏.
 * @return 是否已嵌入
 */
gboolean StatusIcon::IsEmbedded() {
  embedded = gtk_status_icon_is_embedded(statusicon);
  return embedded;
}

/**
 * 改变UI的外观.
 * @return Gtk+库所需
 */
gboolean StatusIcon::onActivate(StatusIcon *self) {
  return self->AlterInterfaceMode();
}

gboolean StatusIcon::AlterInterfaceMode() {
  AlterStatusIconMode();
  if (IsEmbedded()) {
    mwin.AlterWindowMode();
  } else {
    gtk_main_quit();
  }
  return TRUE;
}

}  // namespace iptux
