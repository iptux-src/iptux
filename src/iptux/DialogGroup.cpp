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

#include <glib/gi18n.h>
#include <glog/logging.h>

#include "iptux-core/Const.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"
#include "iptux/DialogPeer.h"
#include "iptux/UiCoreThread.h"
#include "iptux/UiHelper.h"
#include "iptux/callback.h"

using namespace std;

namespace iptux {

/**
 * 类构造函数.
 * @param grp 群组信息
 */
DialogGroup::DialogGroup(Application* app, GroupInfo* grp)
    : DialogBase(CHECK_NOTNULL(app), CHECK_NOTNULL(grp)),
      config(app->getConfig()) {
  InitSublayerSpecify();
}

/**
 * 类析构函数.
 */
DialogGroup::~DialogGroup() {
  SaveUILayout();
}

/**
 * 群组对话框入口.
 * @param grpinf 群组信息
 */
DialogGroup* DialogGroup::GroupDialogEntry(Application* app,
                                           GroupInfo* grpinf) {
  CHECK_NOTNULL(grpinf);
  CHECK_NE(grpinf->getType(), GROUP_BELONG_TYPE_REGULAR);
  DialogGroup* dlggrp;
  GtkWidget *window, *widget;

  dlggrp = new DialogGroup(app, grpinf);
  window = GTK_WIDGET(dlggrp->CreateMainWindow());
  dlggrp->CreateTitle();
  gtk_container_add(GTK_CONTAINER(window), dlggrp->CreateAllArea());
  gtk_widget_show_all(window);

  /* 将焦点置于文本输入框 */
  widget =
      GTK_WIDGET(g_datalist_get_data(&dlggrp->widset, "input-textview-widget"));
  gtk_widget_grab_focus(widget);
  return dlggrp;
}

/**
 * 更新群组成员树(member-tree)指定项.
 * @param pal class PalInfo
 */
void DialogGroup::UpdatePalData(PalInfo* pal) {
  GtkIconTheme* theme;
  GdkPixbuf* pixbuf;
  GtkWidget* widget;
  GtkTreeModel* model;
  GtkTreeIter iter;
  gpointer data;
  gchar* file;

  /* 查询项所在的位置，若没有则添加 */
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "member-treeview-widget"));
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  data = NULL;  // 防止可能出现的(data == pal)
  if (gtk_tree_model_get_iter_first(model, &iter)) {
    do {
      gtk_tree_model_get(model, &iter, 3, &data, -1);
      if (data == pal)
        break;
    } while (gtk_tree_model_iter_next(model, &iter));
  }
  if (data != pal) {
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, FALSE, 3, pal, -1);
  }

  /* 更新数据 */
  theme = gtk_icon_theme_get_default();
  file = iptux_erase_filename_suffix(pal->icon_file().c_str());
  pixbuf = gtk_icon_theme_load_icon(theme, file, MAX_ICONSIZE,
                                    GtkIconLookupFlags(0), NULL);
  g_free(file);
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 1, pixbuf, 2,
                     pal->getName().c_str(), -1);
  if (pixbuf)
    g_object_unref(pixbuf);
}

/**
 * 插入项到群组成员树(member-tree).
 * @param pal class PalInfo
 */
void DialogGroup::InsertPalData(PalInfo* pal) {
  GtkIconTheme* theme;
  GdkPixbuf* pixbuf;
  GtkWidget* widget;
  GtkTreeModel* model;
  GtkTreeIter iter;
  gchar* file;

  theme = gtk_icon_theme_get_default();
  file = iptux_erase_filename_suffix(pal->icon_file().c_str());
  pixbuf = gtk_icon_theme_load_icon(theme, file, MAX_ICONSIZE,
                                    GtkIconLookupFlags(0), NULL);
  g_free(file);
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "member-treeview-widget"));
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  gtk_list_store_append(GTK_LIST_STORE(model), &iter);
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, FALSE, 1, pixbuf, 2,
                     pal->getName().c_str(), 3, pal, -1);
  if (pixbuf)
    g_object_unref(pixbuf);
}

/**
 * 从群组成员树(member-tree)删除指定项.
 * @param pal class PalInfo
 */
void DialogGroup::DelPalData(PalInfo* pal) {
  GtkWidget* widget;
  GtkTreeModel* model;
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
  GtkWidget* widget;
  GtkTreeModel* model;

  widget = GTK_WIDGET(g_datalist_get_data(&widset, "member-treeview-widget"));
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  gtk_list_store_clear(GTK_LIST_STORE(model));
}

/**
 * 初始化底层数据.
 */
void DialogGroup::InitSublayerSpecify() {
  GtkTreeModel* model;
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
GtkWindow* DialogGroup::CreateMainWindow() {
  window = GTK_APPLICATION_WINDOW(gtk_application_window_new(app->getApp()));
  gtk_window_set_title(GTK_WINDOW(window), GetTitle().c_str());
  gtk_window_set_default_size(GTK_WINDOW(window),
                              config->GetInt("group_window_width", 500),
                              config->GetInt("group_window_height", 350));
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  g_datalist_set_data(&widset, "window-widget", window);
  widget_enable_dnd_uri(GTK_WIDGET(window));
  grpinf->setDialogBase(this);

  MainWindowSignalSetup(GTK_WINDOW(window));

  GActionEntry win_entries[] = {
      makeActionEntry("clear_chat_history",
                      G_ACTION_CALLBACK(onClearChatHistory)),
      makeActionEntry("attach_file", G_ACTION_CALLBACK(onAttachFile)),
      makeActionEntry("attach_folder", G_ACTION_CALLBACK(onAttachFolder)),
      makeStateActionEntry("sort_type", G_ACTION_CALLBACK(onSortType), "s",
                           "'ascending'"),
      makeStateActionEntry("sort_by", G_ACTION_CALLBACK(onSortBy), "s",
                           "'nickname'"),
      makeActionEntry("send_message", G_ACTION_CALLBACK(onSendMessage)),
  };
  g_action_map_add_action_entries(G_ACTION_MAP(window), win_entries,
                                  G_N_ELEMENTS(win_entries), this);
  afterWindowCreated();
  return GTK_WINDOW(window);
}

/**
 * 创建所有区域.
 * @return 主窗体
 */
GtkWidget* DialogGroup::CreateAllArea() {
  GtkWidget* box;

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  /* 加入主区域 */
  mainPaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_paned_set_position(GTK_PANED(mainPaned),
                         config->GetInt("group_main_paned_divide", 200));
  gtk_box_pack_start(GTK_BOX(box), mainPaned, TRUE, TRUE, 0);
  g_signal_connect_swapped(mainPaned, "notify::position",
                           G_CALLBACK(onUIChanged), this);

  /* 加入组成员&附件区域 */
  memberEnclosurePaned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
  gtk_paned_set_position(
      GTK_PANED(memberEnclosurePaned),
      config->GetInt("group_memberenclosure_paned_divide", 100));
  gtk_paned_pack1(GTK_PANED(mainPaned), memberEnclosurePaned, FALSE, FALSE);
  g_signal_connect_swapped(memberEnclosurePaned, "notify::position",
                           G_CALLBACK(onUIChanged), this);

  gtk_paned_pack1(GTK_PANED(memberEnclosurePaned), CreateMemberArea(), TRUE,
                  TRUE);
  gtk_paned_pack2(GTK_PANED(memberEnclosurePaned), CreateFileSendArea(), FALSE,
                  TRUE);

  /* 加入聊天历史记录&输入区域 */
  historyInputPaned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
  gtk_paned_set_position(
      GTK_PANED(historyInputPaned),
      config->GetInt("group_historyinput_paned_divide", 100));
  gtk_paned_pack2(GTK_PANED(mainPaned), historyInputPaned, TRUE, TRUE);
  g_signal_connect_swapped(historyInputPaned, "notify::position",
                           G_CALLBACK(onUIChanged), this);

  gtk_paned_pack1(GTK_PANED(historyInputPaned), CreateHistoryArea(), TRUE,
                  TRUE);
  gtk_paned_pack2(GTK_PANED(historyInputPaned), DialogBase::CreateInputArea(),
                  FALSE, TRUE);

  return box;
}

/**
 * 创建组成员区域.
 * @return 主窗体
 */
GtkWidget* DialogGroup::CreateMemberArea() {
  GtkWidget *frame, *sw;
  GtkWidget* widget;
  GtkTreeModel* model;

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
GtkTreeModel* DialogGroup::CreateMemberModel() {
  GtkListStore* model;

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
void DialogGroup::FillMemberModel(GtkTreeModel* model) {
  GtkIconTheme* theme;
  GdkPixbuf* pixbuf;
  GtkTreeIter iter;
  PalInfo* pal;
  char* file;

  theme = gtk_icon_theme_get_default();
  auto g_cthrd = app->getCoreThread();
  g_cthrd->Lock();
  for (auto ppal : grpinf->getMembers()) {
    pal = ppal.get();
    file = iptux_erase_filename_suffix(pal->icon_file().c_str());
    pixbuf = gtk_icon_theme_load_icon(theme, file, MAX_ICONSIZE,
                                      GtkIconLookupFlags(0), NULL);
    g_free(file);
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, TRUE, 1, pixbuf, 2,
                       pal->getName().c_str(), 3, pal, -1);
    if (pixbuf)
      g_object_unref(pixbuf);
  }
  g_cthrd->Unlock();
}

/**
 * 创建群组成员树(member-tree).
 * @param model member-model
 * @return 群组树
 */
GtkWidget* DialogGroup::CreateMemberTree(GtkTreeModel* model) {
  GtkWidget* view;
  GtkTreeSelection* selection;
  GtkCellRenderer* cell;
  GtkTreeViewColumn* column;

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
 * 向选中的好友广播附件消息.
 * @param list 文件链表
 */
void DialogGroup::BroadcastEnclosureMsg(const vector<FileInfo*>& files) {
  GtkWidget* widget;
  GtkTreeModel* model;
  GtkTreeIter iter;
  gboolean active;
  PalInfo* pal;

  vector<const PalInfo*> pals;

  /* 考察是否有成员 */
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "member-treeview-widget"));
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  if (!gtk_tree_model_get_iter_first(model, &iter)) {
    return;
  }

  /* 向选中的成员发送附件 */
  do {
    gtk_tree_model_get(model, &iter, 0, &active, 3, &pal, -1);
    if (active) {
      pals.push_back(pal);
    }
  } while (gtk_tree_model_iter_next(model, &iter));
  app->getCoreThread()->BcstFileInfoEntry(pals, files);
}

/**
 * 向选中的好友广播文本消息.
 * @param msg 文本消息
 */
void DialogGroup::BroadcastTextMsg(const gchar* msg) {
  GtkWidget* widget;
  GtkTreeModel* model;
  GtkTreeIter iter;
  gboolean active;
  uint32_t opttype;
  PalInfo* pal;

  /* 考察是否有成员 */
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "member-treeview-widget"));
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  if (!gtk_tree_model_get_iter_first(model, &iter))
    return;

  /* 向选中的成员发送数据 */
  do {
    gtk_tree_model_get(model, &iter, 0, &active, 3, &pal, -1);
    if (active) {
      if (pal->isCompatible()) {
        switch (grpinf->getType()) {
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
        app->getCoreThread()->SendUnitMessage(pal->GetKey(), opttype, msg);
      } else {
        app->getCoreThread()->SendGroupMessage(pal->GetKey(), msg);
      }
    }
  } while (gtk_tree_model_iter_next(model, &iter));
}

/**
 * 创建选择项的弹出菜单.
 * @param model model
 * @return 菜单
 */
GtkWidget* DialogGroup::CreatePopupMenu(GtkTreeModel* model) {
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
gint DialogGroup::MemberTreeCompareByNameFunc(GtkTreeModel* model,
                                              GtkTreeIter* a,
                                              GtkTreeIter* b) {
  PalInfo *apal, *bpal;
  gint result;

  gtk_tree_model_get(model, a, 3, &apal, -1);
  gtk_tree_model_get(model, b, 3, &bpal, -1);
  result = strcmp(apal->getName().c_str(), bpal->getName().c_str());

  return result;
}

/**
 * 设置群组成员树(member-tree)的比较函数.
 * @param menuitem radio-menu-item
 * @param model member-model
 */
void DialogGroup::SetMemberTreeSortFunc(GtkWidget* menuitem,
                                        GtkTreeModel* model) {
  GtkTreeIterCompareFunc func;

  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
    return;
  func = (GtkTreeIterCompareFunc)(g_object_get_data(G_OBJECT(menuitem),
                                                    "compare-func"));
  gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(model), func, NULL,
                                          NULL);
}

/**
 * 设置群组成员树(member-tree)的排序方式.
 * @param menuitem radio-menu-item
 * @param model member-model
 */
void DialogGroup::SetMemberTreeSortType(GtkWidget* menuitem,
                                        GtkTreeModel* model) {
  GtkSortType type;

  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
    return;
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
gboolean DialogGroup::PopupPickMenu(GtkWidget* treeview,
                                    GdkEventButton* event) {
  GtkWidget* menu;
  GtkTreeModel* model;

  if (!gdk_event_triggers_context_menu((GdkEvent*)event)) {
    return FALSE;
  }
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
  menu = CreatePopupMenu(model);
  gtk_widget_show_all(menu);
  gtk_menu_popup_at_pointer(GTK_MENU(menu), NULL);
  return TRUE;
}

/**
 * 群组成员树(member-tree)项被激活.
 * @param treeview tree-view
 * @param path path
 * @param column column
 */
void DialogGroup::MembertreeItemActivated(GtkWidget* treeview,
                                          GtkTreePath* path,
                                          GtkTreeViewColumn*,
                                          DialogGroup* self) {
  GtkTreeModel* model;
  GtkTreeIter iter;
  PalInfo* pal;
  GroupInfo* grpinf;

  model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
  gtk_tree_model_get_iter(model, &iter, path);
  gtk_tree_model_get(model, &iter, 3, &pal, -1);
  if ((grpinf = self->app->getCoreThread()->GetPalRegularItem(pal))) {
    if ((grpinf->getDialog()))
      gtk_window_present(GTK_WINDOW(grpinf->getDialog()));
    else
      DialogPeer::PeerDialogEntry(self->app, grpinf);
  }
}

bool DialogGroup::SendTextMsg() {
  GtkWidget* textview;
  GtkTextBuffer* buffer;
  GtkTextIter start, end;
  gchar* msg;

  /* 考察缓冲区内是否存在数据 */
  textview = GTK_WIDGET(g_datalist_get_data(&widset, "input-textview-widget"));
  gtk_widget_grab_focus(textview);  // 为下一次任务做准备
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
  gtk_text_buffer_get_bounds(buffer, &start, &end);
  if (gtk_text_iter_equal(&start, &end))
    return false;

  /* 获取数据并发送 */
  msg = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
  gtk_text_buffer_delete(buffer, &start, &end);
  FeedbackMsg(msg);
  BroadcastTextMsg(msg);

  MsgPara msgpara(this->app->getMe());
  msgpara.stype = MessageSourceType::SELF;
  app->getLogSystem()->communicateLog(&msgpara, "[STRING]%s", msg);
  g_free(msg);

  return true;
}

/**
 * 发送消息.
 * @param dlggrp 对话框类
 */
void DialogGroup::SendMessage(DialogGroup* dlggrp) {
  dlggrp->SendEnclosureMsg();
  dlggrp->SendTextMsg();
}
/**
 * 获取待发送成员列表.
 * @return plist 获取待发送成员列表
 * 调用该函数后须free plist
 */
GSList* DialogGroup::GetSelPal() {
  GtkWidget* widget;
  GtkTreeModel* model;
  GtkTreeIter iter;
  gboolean active;
  PalInfo* pal;
  GSList* plist;

  /* 考察是否有成员 */
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "member-treeview-widget"));
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  if (!gtk_tree_model_get_iter_first(model, &iter))
    return NULL;

  plist = NULL;
  do {
    gtk_tree_model_get(model, &iter, 0, &active, 3, &pal, -1);
    if (active)
      plist = g_slist_append(plist, pal);
  } while (gtk_tree_model_iter_next(model, &iter));
  return plist;
}

void DialogGroup::onUIChanged(DialogGroup& self) {
  self.config->SetInt("group_main_paned_divide",
                      gtk_paned_get_position(GTK_PANED(self.mainPaned)));
  self.config->SetInt(
      "group_memberenclosure_paned_divide",
      gtk_paned_get_position(GTK_PANED(self.memberEnclosurePaned)));
  self.config->SetInt(
      "group_historyinput_paned_divide",
      gtk_paned_get_position(GTK_PANED(self.historyInputPaned)));
  self.config->Save();
}

void DialogGroup::onSortBy(GSimpleAction* action,
                           GVariant* value,
                           DialogGroup& self) {
  string sortBy = g_variant_get_string(value, nullptr);

  PalTreeModelSortKey key;

  if (sortBy == "nickname") {
    key = PalTreeModelSortKey::NICKNAME;
  } else if (sortBy == "ip") {
    key = PalTreeModelSortKey::IP;
  } else {
    LOG_WARN("unknown sort by: %s", sortBy.c_str());
    return;
  }

  auto model =
      GTK_TREE_MODEL(g_datalist_get_data(&self.mdlset, "member-model"));
  palTreeModelSetSortKey(model, key);
  g_simple_action_set_state(action, value);
}

void DialogGroup::onSortType(GSimpleAction* action,
                             GVariant* value,
                             DialogGroup& self) {
  string sortType = g_variant_get_string(value, nullptr);

  GtkSortType type;

  if (sortType == "ascending") {
    type = GTK_SORT_ASCENDING;
  } else if (sortType == "descending") {
    type = GTK_SORT_DESCENDING;
  } else {
    LOG_WARN("unknown sorttype: %s", sortType.c_str());
    return;
  }

  auto model =
      GTK_TREE_MODEL(g_datalist_get_data(&self.mdlset, "member-model"));
  gtk_tree_sortable_set_sort_column_id(
      GTK_TREE_SORTABLE(model), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, type);
  g_simple_action_set_state(action, value);
}

void DialogGroup::CreateTitle() {
  if (app->use_header_bar()) {
    GtkHeaderBar* header_bar = CreateHeaderBar(GTK_WINDOW(window), app->menu());
    gtk_header_bar_set_title(header_bar, GetTitle().c_str());
  }
}

string DialogGroup::GetTitle() {
  return stringFormat(_("Talk with the group %s"), grpinf->name().c_str());
}

}  // namespace iptux
