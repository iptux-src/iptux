//
// C++ Implementation: DialogGroup
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "config.h"
#include "DialogGroup.h"

#include "iptux-core/deplib.h"
#include "iptux-core/ipmsg.h"
#include "iptux-utils/utils.h"
#include "iptux-utils/output.h"
#include "iptux/callback.h"
#include "iptux/DialogPeer.h"
#include "iptux/global.h"
#include "iptux/HelpDialog.h"
#include "iptux/UiHelper.h"

using namespace std;

namespace iptux {

/**
 * 类构造函数.
 * @param grp 群组信息
 */
DialogGroup::DialogGroup(MainWindow* mainWindow, GroupInfo *grp,
                         shared_ptr<UiProgramData> progdt)
    : DialogBase(grp, progdt),
      mainWindow(mainWindow),
      config(mainWindow->getConfig()) {
  InitSublayerSpecify();
}

/**
 * 类析构函数.
 */
DialogGroup::~DialogGroup() {
  mainWindow->clearActiveWindow(this);
  SaveUILayout();
}

/**
 * 群组对话框入口.
 * @param grpinf 群组信息
 */
void DialogGroup::GroupDialogEntry(MainWindow* mainWindow, GroupInfo *grpinf,
                                   shared_ptr<UiProgramData> progdt) {
  DialogGroup *dlggrp;
  GtkWidget *window, *widget;

  dlggrp = new DialogGroup(mainWindow, grpinf, progdt);
  window = GTK_WIDGET(dlggrp->CreateMainWindow());
  gtk_container_add(GTK_CONTAINER(window), dlggrp->CreateAllArea());
  gtk_widget_show_all(window);

  /* 将焦点置于文本输入框 */
  widget =
      GTK_WIDGET(g_datalist_get_data(&dlggrp->widset, "input-textview-widget"));
  gtk_widget_grab_focus(widget);
  /* 从消息队列中移除 */
  g_cthrd->Lock();
  if (g_cthrd->MsglineContainItem(grpinf)) {
    g_mwin->MakeItemBlinking(grpinf, FALSE);
    g_cthrd->PopItemFromMsgline(grpinf);
  }
  g_cthrd->Unlock();

  /* delete dlggrp;//请不要这样做，此类将会在窗口被摧毁后自动释放 */
}

/**
 * 更新群组成员树(member-tree)指定项.
 * @param pal class PalInfo
 */
void DialogGroup::UpdatePalData(PalInfo *pal) {
  GtkIconTheme *theme;
  GdkPixbuf *pixbuf;
  GtkWidget *widget;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gpointer data;
  gchar *file;

  /* 查询项所在的位置，若没有则添加 */
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "member-treeview-widget"));
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  data = NULL;  //防止可能出现的(data == pal)
  if (gtk_tree_model_get_iter_first(model, &iter)) {
    do {
      gtk_tree_model_get(model, &iter, 3, &data, -1);
      if (data == pal) break;
    } while (gtk_tree_model_iter_next(model, &iter));
  }
  if (data != pal) {
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, FALSE, 3, pal, -1);
  }

  /* 更新数据 */
  theme = gtk_icon_theme_get_default();
  file = iptux_erase_filename_suffix(pal->iconfile);
  pixbuf = gtk_icon_theme_load_icon(theme, file, MAX_ICONSIZE,
                                    GtkIconLookupFlags(0), NULL);
  g_free(file);
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 1, pixbuf, 2, pal->name, -1);
  if (pixbuf) g_object_unref(pixbuf);
}

/**
 * 插入项到群组成员树(member-tree).
 * @param pal class PalInfo
 */
void DialogGroup::InsertPalData(PalInfo *pal) {
  GtkIconTheme *theme;
  GdkPixbuf *pixbuf;
  GtkWidget *widget;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *file;

  theme = gtk_icon_theme_get_default();
  file = iptux_erase_filename_suffix(pal->iconfile);
  pixbuf = gtk_icon_theme_load_icon(theme, file, MAX_ICONSIZE,
                                    GtkIconLookupFlags(0), NULL);
  g_free(file);
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "member-treeview-widget"));
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  gtk_list_store_append(GTK_LIST_STORE(model), &iter);
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, FALSE, 1, pixbuf, 2,
                     pal->name, 3, pal, -1);
  if (pixbuf) g_object_unref(pixbuf);
}

/**
 * 从群组成员树(member-tree)删除指定项.
 * @param pal class PalInfo
 */
void DialogGroup::DelPalData(PalInfo *pal) {
  GtkWidget *widget;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gpointer data;

  widget = GTK_WIDGET(g_datalist_get_data(&widset, "member-treeview-widget"));
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  if (gtk_tree_model_get_iter_first(model, &iter)) {
    do {
      gtk_tree_model_get(model, &iter, 3, &data, -1);
      if (data == pal) {
        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
        break;
      }
    } while (gtk_tree_model_iter_next(model, &iter));
  }
}

/**
 * 清除本群组所有好友数据.
 */
void DialogGroup::ClearAllPalData() {
  GtkWidget *widget;
  GtkTreeModel *model;

  widget = GTK_WIDGET(g_datalist_get_data(&widset, "member-treeview-widget"));
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  gtk_list_store_clear(GTK_LIST_STORE(model));
}

/**
 * 初始化底层数据.
 */
void DialogGroup::InitSublayerSpecify() {
  GtkTreeModel *model;
  model = CreateMemberModel();
  g_datalist_set_data_full(&mdlset, "member-model", model,
                           GDestroyNotify(g_object_unref));
  FillMemberModel(model);
}

/**
 * 写出对话框的UI布局数据.
 */
void DialogGroup::SaveUILayout() {
  config->SetInt("group_window_width",
                GPOINTER_TO_INT(g_datalist_get_data(&dtset, "window-width")));
  config->SetInt("group_window_height",
                GPOINTER_TO_INT(g_datalist_get_data(&dtset, "window-height")));
}

/**
 * 创建主窗口.
 * @return 窗口
 */
GtkWindow *DialogGroup::CreateMainWindow() {
  char buf[MAX_BUFLEN];
  window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
  snprintf(buf, MAX_BUFLEN, _("Talk with the group %s"), grpinf->name.c_str());
  gtk_window_set_title(GTK_WINDOW(window), buf);
  gtk_window_set_default_size(GTK_WINDOW(window),
                              config->GetInt("group_window_width", 500),
                              config->GetInt("group_window_height", 350));
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_add_accel_group(GTK_WINDOW(window), accel);
  g_datalist_set_data(&widset, "window-widget", window);
  widget_enable_dnd_uri(GTK_WIDGET(window));
  grpinf->dialog = GTK_WIDGET(window);

  MainWindowSignalSetup(window);
  g_signal_connect_swapped(window, "notify::is-active", G_CALLBACK(onActive), this);

  return window;
}

/**
 * 创建所有区域.
 * @return 主窗体
 */
GtkWidget *DialogGroup::CreateAllArea() {
  GtkWidget *box;

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  /* 加入菜单条 */
  gtk_box_pack_start(GTK_BOX(box), CreateMenuBar(), FALSE, FALSE, 0);

  /* 加入主区域 */
  mainPaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_paned_set_position(GTK_PANED(mainPaned), config->GetInt("group_main_paned_divide", 200));
  gtk_box_pack_start(GTK_BOX(box), mainPaned, TRUE, TRUE, 0);
  g_signal_connect_swapped(mainPaned, "notify::position", G_CALLBACK(onUIChanged), this);

  /* 加入组成员&附件区域 */
  memberEnclosurePaned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
  gtk_paned_set_position(GTK_PANED(memberEnclosurePaned), config->GetInt("group_memberenclosure_paned_divide", 100));
  gtk_paned_pack1(GTK_PANED(mainPaned), memberEnclosurePaned, FALSE, TRUE);
  g_signal_connect_swapped(memberEnclosurePaned, "notify::position", G_CALLBACK(onUIChanged), this);

  gtk_paned_pack1(GTK_PANED(memberEnclosurePaned), CreateMemberArea(), TRUE, TRUE);
  gtk_paned_pack2(GTK_PANED(memberEnclosurePaned), CreateFileSendArea(), FALSE, TRUE);

  /* 加入聊天历史记录&输入区域 */
  historyInputPaned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
  gtk_paned_set_position(GTK_PANED(historyInputPaned), config->GetInt("group_historyinput_paned_divide", 100));
  gtk_paned_pack2(GTK_PANED(mainPaned), historyInputPaned, TRUE, TRUE);
  g_signal_connect_swapped(historyInputPaned, "notify::position", G_CALLBACK(onUIChanged), this);

  gtk_paned_pack1(GTK_PANED(historyInputPaned), CreateHistoryArea(), TRUE, TRUE);
  gtk_paned_pack2(GTK_PANED(historyInputPaned), DialogBase::CreateInputArea(), FALSE,
                  TRUE);

  return box;
}

/**
 * 创建菜单条.
 * @return 菜单条
 */
GtkWidget *DialogGroup::CreateMenuBar() {
  GtkWidget *menubar;

  menubar = gtk_menu_bar_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), CreateFileMenu());
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), CreateToolMenu());

  return menubar;
}

/**
 * 创建组成员区域.
 * @return 主窗体
 */
GtkWidget *DialogGroup::CreateMemberArea() {
  GtkWidget *frame, *sw;
  GtkWidget *widget;
  GtkTreeModel *model;

  frame = gtk_frame_new(_("Member"));
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
  sw = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                      GTK_SHADOW_ETCHED_IN);
  gtk_container_add(GTK_CONTAINER(frame), sw);

  model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "member-model"));
  widget = CreateMemberTree(model);
  gtk_container_add(GTK_CONTAINER(sw), widget);
  g_signal_connect(widget, "button-press-event", G_CALLBACK(PopupPickMenu),
                   NULL);
  g_signal_connect(widget, "row-activated", G_CALLBACK(MembertreeItemActivated),
                   this);
  g_datalist_set_data(&widset, "member-treeview-widget", widget);

  return frame;
}

/**
 * 群组成员树(member-tree)底层数据结构.
 * 4,0 toggled,1 icon,2 nickname,3 data \n
 * 是否被选中;好友头像;好友昵称;好友数据 \n
 * @return member-model
 */
GtkTreeModel *DialogGroup::CreateMemberModel() {
  GtkListStore *model;

  model = gtk_list_store_new(4, G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF, G_TYPE_STRING,
                             G_TYPE_POINTER);
  gtk_tree_sortable_set_default_sort_func(
      GTK_TREE_SORTABLE(model),
      GtkTreeIterCompareFunc(MemberTreeCompareByNameFunc), NULL, NULL);
  gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model),
                                       GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
                                       GTK_SORT_ASCENDING);

  return GTK_TREE_MODEL(model);
}

/**
 * 为群组成员树(member-tree)填充底层数据.
 * @param model member-model
 */
void DialogGroup::FillMemberModel(GtkTreeModel *model) {
  GtkIconTheme *theme;
  GdkPixbuf *pixbuf;
  GtkTreeIter iter;
  GSList *tlist;
  PalInfo *pal;
  char *file;

  theme = gtk_icon_theme_get_default();
  g_cthrd->Lock();
  tlist = grpinf->member;
  while (tlist) {
    pal = (PalInfo *)tlist->data;
    file = iptux_erase_filename_suffix(pal->iconfile);
    pixbuf = gtk_icon_theme_load_icon(theme, file, MAX_ICONSIZE,
                                      GtkIconLookupFlags(0), NULL);
    g_free(file);
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, TRUE, 1, pixbuf, 2,
                       pal->name, 3, pal, -1);
    if (pixbuf) g_object_unref(pixbuf);
    tlist = g_slist_next(tlist);
  }
  g_cthrd->Unlock();
}

/**
 * 创建群组成员树(member-tree).
 * @param model member-model
 * @return 群组树
 */
GtkWidget *DialogGroup::CreateMemberTree(GtkTreeModel *model) {
  GtkWidget *view;
  GtkTreeSelection *selection;
  GtkCellRenderer *cell;
  GtkTreeViewColumn *column;

  view = gtk_tree_view_new_with_model(model);
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), TRUE);
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
  gtk_tree_selection_set_mode(selection, GTK_SELECTION_NONE);

  cell = gtk_cell_renderer_toggle_new();
  g_signal_connect_swapped(cell, "toggled", G_CALLBACK(model_turn_select),
                           model);
  column = gtk_tree_view_column_new_with_attributes(_("Send"), cell, "active",
                                                    0, NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  column = gtk_tree_view_column_new();
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_column_set_title(column, _("Pals"));
  cell = gtk_cell_renderer_pixbuf_new();
  gtk_tree_view_column_pack_start(column, cell, FALSE);
  gtk_tree_view_column_set_attributes(column, cell, "pixbuf", 1, NULL);
  cell = gtk_cell_renderer_text_new();
  gtk_tree_view_column_pack_start(column, cell, FALSE);
  gtk_tree_view_column_set_attributes(column, cell, "text", 2, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  return view;
}

/**
 * 创建工具菜单.
 * @return 菜单
 */
GtkWidget *DialogGroup::CreateToolMenu() {
  GtkWidget *menushell;
  GtkWidget *menu, *submenu, *menuitem;
  GtkTreeModel *model;

  model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "member-model"));
  menushell = gtk_menu_item_new_with_mnemonic(_("_Tools"));
  menu = gtk_menu_new();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menushell), menu);

  /* 群组成员排序 */
  NO_OPERATION_C
  menuitem = gtk_menu_item_new_with_label(_("Sort"));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
  submenu = gtk_menu_new();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), submenu);
  /*/* 按昵称排序 */
  NO_OPERATION_C
  menuitem = gtk_radio_menu_item_new_with_label(NULL, _("By Nickname"));
  g_object_set_data(G_OBJECT(menuitem), "compare-func",
                    (gpointer)MemberTreeCompareByNameFunc);
  gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
  g_signal_connect(menuitem, "toggled", G_CALLBACK(SetMemberTreeSortFunc),
                   model);
  /*/* 按IP地址排序 */
  menuitem = gtk_radio_menu_item_new_with_label_from_widget(
      GTK_RADIO_MENU_ITEM(menuitem), _("By IP"));
  g_object_set_data(G_OBJECT(menuitem), "compare-func",
                    (gpointer)MemberTreeCompareByIPFunc);
  gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
  g_signal_connect(menuitem, "toggled", G_CALLBACK(SetMemberTreeSortFunc),
                   model);
  /*/* 分割符 */
  menuitem = gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
  /*/* 升序 */
  NO_OPERATION_C
  menuitem = gtk_radio_menu_item_new_with_label(NULL, _("Ascending"));
  g_object_set_data(G_OBJECT(menuitem), "sort-type",
                    GINT_TO_POINTER(GTK_SORT_ASCENDING));
  gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
  g_signal_connect(menuitem, "toggled", G_CALLBACK(SetMemberTreeSortType),
                   model);
  /*/* 降序 */
  menuitem = gtk_radio_menu_item_new_with_label_from_widget(
      GTK_RADIO_MENU_ITEM(menuitem), _("Descending"));
  g_object_set_data(G_OBJECT(menuitem), "sort-type",
                    GINT_TO_POINTER(GTK_SORT_DESCENDING));
  gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
  g_signal_connect(menuitem, "toggled", G_CALLBACK(SetMemberTreeSortType),
                   model);

  return menushell;
}

/**
 * 向选中的好友广播附件消息.
 * @param list 文件链表
 */
void DialogGroup::BroadcastEnclosureMsg(GSList *list) {
  GtkWidget *widget;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gboolean active;
  PalInfo *pal;
  GSList *plist;

  /* 考察是否有成员 */
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "member-treeview-widget"));
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  if (!gtk_tree_model_get_iter_first(model, &iter)) {
    /**
     * @note 链表(list)的数据本来应该由(sfile.BcstFileInfoEntry())接手的，
     * 既然已经没有那个机会了， 当然就只好在这儿手动释放了.
     */
    g_slist_foreach(list, GFunc(g_free), NULL);
    return;
  }

  /* 向选中的成员发送附件 */
  plist = NULL;
  do {
    gtk_tree_model_get(model, &iter, 0, &active, 3, &pal, -1);
    if (active) plist = g_slist_append(plist, pal);
  } while (gtk_tree_model_iter_next(model, &iter));
  g_cthrd->BcstFileInfoEntry(plist, list);
  g_slist_free(plist);
}

/**
 * 向选中的好友广播文本消息.
 * @param msg 文本消息
 */
void DialogGroup::BroadcastTextMsg(const gchar *msg) {
  GtkWidget *widget;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gboolean active;
  uint32_t opttype;
  PalInfo *pal;

  /* 考察是否有成员 */
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "member-treeview-widget"));
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  if (!gtk_tree_model_get_iter_first(model, &iter)) return;

  /* 向选中的成员发送数据 */
  do {
    gtk_tree_model_get(model, &iter, 0, &active, 3, &pal, -1);
    if (active) {
      if (pal->isCompatible()) {
        switch (grpinf->type) {
          case GROUP_BELONG_TYPE_BROADCAST:
            opttype = IPTUX_BROADCASTOPT;
            break;
          case GROUP_BELONG_TYPE_GROUP:
            opttype = IPTUX_GROUPOPT;
            break;
          case GROUP_BELONG_TYPE_SEGMENT:
            opttype = IPTUX_SEGMENTOPT;
            break;
          case GROUP_BELONG_TYPE_REGULAR:
          default:
            opttype = IPTUX_REGULAROPT;
            break;
        }
        g_cthrd->SendUnitMessage(pal->GetKey(), opttype, msg);
      } else {
        g_cthrd->SendGroupMessage(pal->GetKey(), msg);
      }
    }
  } while (gtk_tree_model_iter_next(model, &iter));
}

/**
 * 创建选择项的弹出菜单.
 * @param model model
 * @return 菜单
 */
GtkWidget *DialogGroup::CreatePopupMenu(GtkTreeModel *model) {
  GtkWidget *menu, *menuitem;

  menu = gtk_menu_new();

  menuitem = gtk_menu_item_new_with_label(_("Select All"));
  g_signal_connect_swapped(menuitem, "activate", G_CALLBACK(model_select_all),
                           model);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

  menuitem = gtk_menu_item_new_with_label(_("Reverse Select"));
  g_signal_connect_swapped(menuitem, "activate", G_CALLBACK(model_turn_all),
                           model);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

  menuitem = gtk_menu_item_new_with_label(_("Clear Up"));
  g_signal_connect_swapped(menuitem, "activate", G_CALLBACK(model_clear_all),
                           model);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

  return menu;
}

/**
 * 群组成员树(member-tree)按昵称排序的比较函数.
 * @param model member-model
 * @param a A GtkTreeIter in model
 * @param b Another GtkTreeIter in model
 * @return 比较值
 */
gint DialogGroup::MemberTreeCompareByNameFunc(GtkTreeModel *model,
                                              GtkTreeIter *a, GtkTreeIter *b) {
  PalInfo *apal, *bpal;
  gint result;

  gtk_tree_model_get(model, a, 3, &apal, -1);
  gtk_tree_model_get(model, b, 3, &bpal, -1);
  result = strcmp(apal->name, bpal->name);

  return result;
}

/**
 * 群组成员树(member-tree)按IP排序的比较函数.
 * @param model member-model
 * @param a A GtkTreeIter in model
 * @param b Another GtkTreeIter in model
 * @return 比较值
 */
gint DialogGroup::MemberTreeCompareByIPFunc(GtkTreeModel *model, GtkTreeIter *a,
                                            GtkTreeIter *b) {
  PalInfo *apal, *bpal;

  gtk_tree_model_get(model, a, 3, &apal, -1);
  gtk_tree_model_get(model, b, 3, &bpal, -1);
  return ipv4Compare(apal->ipv4, bpal->ipv4);
}

/**
 * 设置群组成员树(member-tree)的比较函数.
 * @param menuitem radio-menu-item
 * @param model member-model
 */
void DialogGroup::SetMemberTreeSortFunc(GtkWidget *menuitem,
                                        GtkTreeModel *model) {
  GtkTreeIterCompareFunc func;

  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem))) return;
  func = (GtkTreeIterCompareFunc)(
      g_object_get_data(G_OBJECT(menuitem), "compare-func"));
  gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(model), func, NULL,
                                          NULL);
}

/**
 * 设置群组成员树(member-tree)的排序方式.
 * @param menuitem radio-menu-item
 * @param model member-model
 */
void DialogGroup::SetMemberTreeSortType(GtkWidget *menuitem,
                                        GtkTreeModel *model) {
  GtkSortType type;

  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem))) return;
  type = (GtkSortType)GPOINTER_TO_INT(
      g_object_get_data(G_OBJECT(menuitem), "sort-type"));
  gtk_tree_sortable_set_sort_column_id(
      GTK_TREE_SORTABLE(model), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, type);
}

/**
 * 弹出选择项的菜单.
 * @param treeview tree-view
 * @param event event
 * @return Gtk+库所需
 */
gboolean DialogGroup::PopupPickMenu(GtkWidget *treeview,
                                    GdkEventButton *event) {
  GtkWidget *menu;
  GtkTreeModel *model;

  if (event->button != GDK_BUTTON_SECONDARY) {
    return FALSE;
  }
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
  menu = CreatePopupMenu(model);
  gtk_widget_show_all(menu);
  gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event->button,
                 event->time);

  return TRUE;
}

/**
 * 群组成员树(member-tree)项被激活.
 * @param treeview tree-view
 * @param path path
 * @param column column
 */
void DialogGroup::MembertreeItemActivated(GtkWidget *treeview,
                                          GtkTreePath *path,
                                          GtkTreeViewColumn *column,
                                          DialogGroup *self) {
  GtkTreeModel *model;
  GtkTreeIter iter;
  PalInfo *pal;
  GroupInfo *grpinf;

  model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
  gtk_tree_model_get_iter(model, &iter, path);
  gtk_tree_model_get(model, &iter, 3, &pal, -1);
  if ((grpinf = g_cthrd->GetPalRegularItem(pal))) {
    if ((grpinf->dialog))
      gtk_window_present(GTK_WINDOW(grpinf->dialog));
    else
      DialogPeer::PeerDialogEntry(g_mwin, grpinf, self->progdt);
  }
}

bool DialogGroup::SendTextMsg() {
  GtkWidget *textview;
  GtkTextBuffer *buffer;
  GtkTextIter start, end;
  MsgPara msgpara;
  gchar *msg;

  /* 考察缓冲区内是否存在数据 */
  textview = GTK_WIDGET(g_datalist_get_data(&widset, "input-textview-widget"));
  gtk_widget_grab_focus(textview);  //为下一次任务做准备
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
  gtk_text_buffer_get_bounds(buffer, &start, &end);
  if (gtk_text_iter_equal(&start, &end)) return false;

  /* 获取数据并发送 */
  msg = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
  gtk_text_buffer_delete(buffer, &start, &end);
  FeedbackMsg(msg);
  BroadcastTextMsg(msg);

  msgpara.stype = MessageSourceType::SELF;
  msgpara.pal = NULL;
  g_cthrd->CommunicateLog(&msgpara, "[STRING]%s", msg);
  g_free(msg);

  return true;
}

/**
 * 发送消息.
 * @param dlggrp 对话框类
 */
void DialogGroup::SendMessage(DialogGroup *dlggrp) {
  dlggrp->SendEnclosureMsg();
  dlggrp->SendTextMsg();
}
/**
 * 获取待发送成员列表.
 * @return plist 获取待发送成员列表
 * 调用该函数后须free plist
 */
GSList *DialogGroup::GetSelPal() {
  GtkWidget *widget;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gboolean active;
  PalInfo *pal;
  GSList *plist;

  /* 考察是否有成员 */
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "member-treeview-widget"));
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  if (!gtk_tree_model_get_iter_first(model, &iter)) return NULL;

  plist = NULL;
  do {
    gtk_tree_model_get(model, &iter, 0, &active, 3, &pal, -1);
    if (active) plist = g_slist_append(plist, pal);
  } while (gtk_tree_model_iter_next(model, &iter));
  return plist;
}

void DialogGroup::onUIChanged(DialogGroup &self) {
  self.config->SetInt("group_main_paned_divide", gtk_paned_get_position(GTK_PANED(self.mainPaned)));
  self.config->SetInt("group_memberenclosure_paned_divide", gtk_paned_get_position(GTK_PANED(self.memberEnclosurePaned)));
  self.config->SetInt("group_historyinput_paned_divide", gtk_paned_get_position(GTK_PANED(self.historyInputPaned)));
  self.config->Save();
}

void DialogGroup::onActive(DialogGroup& self) {
  if(!gtk_window_is_active(GTK_WINDOW(self.window))) {
    return;
  }
  self.mainWindow->setActiveWindow(ActiveWindowType::GROUP, &self);
}

}  // namespace iptux
