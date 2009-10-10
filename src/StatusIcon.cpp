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
#include "ProgramData.h"
#include "CoreThread.h"
#include "MainWindow.h"
#include "DialogPeer.h"
#include "DialogGroup.h"
#include "DataSettings.h"
#include "ShareFile.h"
#include "DetectPal.h"
#include "callback.h"
#include "support.h"
#include "utils.h"
extern ProgramData progdt;
extern CoreThread cthrd;
extern MainWindow mwin;

/**
 * 类构造函数.
 */
StatusIcon::StatusIcon():statusicon(NULL), timerid(0)
{
}

/**
 * 类析构函数.
 */
StatusIcon::~StatusIcon()
{
	g_object_unref(statusicon);
	g_source_remove(timerid);
}

/**
 * 创建状态图标.
 */
void StatusIcon::CreateStatusIcon()
{
	GdkScreen *screen;

	if (FLAG_ISSET(progdt.flags, 6)) {
		statusicon = gtk_status_icon_new_from_stock("iptux-logo-hide");
		g_object_set_data(G_OBJECT(statusicon), "show", GINT_TO_POINTER(FALSE));
	} else {
		statusicon = gtk_status_icon_new_from_stock("iptux-logo-show");
		g_object_set_data(G_OBJECT(statusicon), "show", GINT_TO_POINTER(TRUE));
	}
	screen = gdk_screen_get_default();
	gtk_status_icon_set_screen(statusicon, screen);
	gtk_status_icon_set_tooltip(statusicon, _("iptux"));

	g_signal_connect(statusicon, "activate", G_CALLBACK(StatusIconActivate), NULL);
	g_signal_connect(statusicon, "popup-menu", G_CALLBACK(PopupWorkMenu), NULL);
	timerid = gdk_threads_add_timeout(1000, GSourceFunc(UpdateUI), this);
}

/**
 * 更改状态图标的表现形式.
 */
void StatusIcon::AlterStatusIconMode()
{
	if (g_object_get_data(G_OBJECT(statusicon), "show")) {
		gtk_status_icon_set_from_stock(statusicon, "iptux-logo-hide");
		g_object_set_data(G_OBJECT(statusicon), "show", GINT_TO_POINTER(FALSE));
	} else {
		gtk_status_icon_set_from_stock(statusicon, "iptux-logo-show");
		g_object_set_data(G_OBJECT(statusicon), "show", GINT_TO_POINTER(TRUE));
	}
}

/**
 * 更新与状态图标相关的UI.
 * @param sicon 状态图标类
 * @return GLib库所需
 */
gboolean StatusIcon::UpdateUI(StatusIcon *sicon)
{
	char *ipstr;
	guint len;

	pthread_mutex_lock(cthrd.GetMutex());
	if ( (len = cthrd.GetMsglineItems())) {
		gtk_status_icon_set_blinking(sicon->statusicon, TRUE);
		ipstr = g_strdup_printf(_("To be read: %u messages"), len);
		gtk_status_icon_set_tooltip(sicon->statusicon, ipstr);
	} else {
		gtk_status_icon_set_blinking(sicon->statusicon, FALSE);
		ipstr = get_sys_host_addr_string(cthrd.UdpSockQuote());
		ipstr = ipstr ? ipstr : g_strdup(_("iptux"));
		gtk_status_icon_set_tooltip(sicon->statusicon, ipstr);
	}
	g_free(ipstr);
	pthread_mutex_unlock(cthrd.GetMutex());

	return TRUE;
}

/**
 * 创建弹出菜单.
 * @return 菜单
 */
GtkWidget *StatusIcon::CreatePopupMenu(GtkStatusIcon *statusicon)
{
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
	g_signal_connect(menuitem, "activate", G_CALLBACK(alter_interface_mode), NULL);

	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	/* 显示文件传输窗口 */
	NO_OPERATION_C
	menuitem = gtk_image_menu_item_new_with_mnemonic(_("_Transmission"));
	image = gtk_image_new_from_stock(GTK_STOCK_CONNECT, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(ShowTransWindow), NULL);

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
void StatusIcon::ShowTransWindow()
{
	mwin.OpenTransWindow();
}

/**
 * 状态图标被激活的响应处理函数.
 */
void StatusIcon::StatusIconActivate()
{
	GroupInfo *grpinf;

	pthread_mutex_lock(cthrd.GetMutex());
	if (cthrd.GetMsglineItems())
		grpinf = cthrd.GetMsglineHeadItem();
	else
		grpinf = NULL;
	pthread_mutex_unlock(cthrd.GetMutex());
	if (grpinf) {
		switch (grpinf->type) {
		case REGULAR_TYPE:
			DialogPeer::PeerDialogEntry(grpinf);
			break;
		case SEGMENT_TYPE:
		case GROUP_TYPE:
		case BROADCAST_TYPE:
			DialogGroup::GroupDialogEntry(grpinf);
			break;
		default:
			break;
		}
	} else
		alter_interface_mode();
}

/**
 * 弹出菜单.
 * @param statusicon the object which received the signal
 * @param button the button that was pressed
 * @param time the timestamp of the event that triggered the signal emission
 */
void StatusIcon::PopupWorkMenu(GtkStatusIcon *statusicon, guint button, guint time)
{
	GtkWidget *menu;

	menu = CreatePopupMenu(statusicon);
	gtk_widget_show_all(menu);
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, button, time);
}
