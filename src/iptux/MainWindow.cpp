//
// C++ Implementation: MainWindow
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
#include "MainWindow.h"

#include <string>
#include <cinttypes>
#include <glog/logging.h>
#include <glib/gi18n.h>

#include "iptux-core/Const.h"
#include "iptux-utils/utils.h"
#include "iptux-utils/output.h"
#include "iptux/callback.h"
#include "iptux/DataSettings.h"
#include "iptux/DetectPal.h"
#include "iptux/dialog.h"
#include "iptux/DialogGroup.h"
#include "iptux/DialogPeer.h"
#include "iptux/global.h"
#include "iptux/HelpDialog.h"
#include "iptux/RevisePal.h"
#include "iptux/ShareFile.h"
#include "iptux/StatusIcon.h"
#include "iptux/TransWindow.h"
#include "iptux/UiHelper.h"
#include "iptux/UiModels.h"

using namespace std;

namespace iptux {

static gchar* palInfo2HintMarkup(const PalInfo *pal);


/**
 * 类构造函数.
 */
MainWindow::MainWindow(Application* app, UiCoreThread& coreThread)
    : app(app),
      coreThread(coreThread),
      window(nullptr),
      progdt(coreThread.getUiProgramData()),
      config(progdt->getConfig()),
      widset(NULL),
      mdlset(NULL),
      tmdllist(NULL),
      accel(NULL),
      timerid(0),
      windowConfig(250, 510, "main_window"),
      palPopupMenu(0)
{
  transWindow = nullptr;
  windowConfig.LoadFromConfig(config);
  builder = gtk_builder_new_from_file(__UI_PATH "/main.ui");
  gtk_builder_connect_signals(builder, nullptr);
  g_cthrd->registerCallback([&](shared_ptr<const Event> event) { this->processEvent(event); });
}

/**
 * 类析构函数.
 */
MainWindow::~MainWindow() {
  ClearSublayer();
}

GtkWidget* MainWindow::getWindow() {
  return window;
}

/**
 * 创建程序主窗口入口.
 */
void MainWindow::CreateWindow() {
  GtkWidget *widget;

  InitSublayer();

  /* 创建主窗口 */
  window = CreateMainWindow();
  g_object_set_data(G_OBJECT(window), "iptux-config", &config);
  g_object_set_data(G_OBJECT(window), "trans-model", app->getTransModel());

  gtk_container_add(GTK_CONTAINER(window), CreateAllArea());
  gtk_widget_show_all(window);


  GActionEntry win_entries[] = {
      { "refresh", G_ACTION_CALLBACK(onRefresh)},
      { "sort_type", nullptr, "s", "'ascending'", G_ACTION_CALLBACK(onSortType)},
      { "sort_by", nullptr, "s", "'nickname'", G_ACTION_CALLBACK(onSortBy)},
      { "detect", G_ACTION_CALLBACK(onDetect)},
      { "find", G_ACTION_CALLBACK(onFind)},
      { "pal.send_message", G_ACTION_CALLBACK(onPalSendMessage)},
      { "pal.request_shared_resources", G_ACTION_CALLBACK(onPalRequestSharedResources)},
      { "pal.change_info", G_ACTION_CALLBACK(onPalChangeInfo)},
      { "pal.delete_pal", G_ACTION_CALLBACK(onDeletePal)},
  };

  g_action_map_add_action_entries (G_ACTION_MAP (window),
                                   win_entries, G_N_ELEMENTS (win_entries),
                                   this);
  /* 聚焦到好友树(paltree)区域 */
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "paltree-treeview-widget"));
  gtk_widget_grab_focus(widget);
  /* 隐藏好友清单 */
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "pallist-box-widget"));
  gtk_widget_hide(widget);

  if (progdt->IsAutoHidePanelAfterLogin()) {
    gtk_widget_hide(window);
  }

  palPopupMenu = GTK_MENU(gtk_menu_new_from_model(
    G_MENU_MODEL(gtk_builder_get_object(app->getMenuBuilder(), "pal-popup"))
  ));
  gtk_menu_attach_to_widget(palPopupMenu, window, nullptr);
}

/**
 * 更改窗口显示模式.
 */
void MainWindow::AlterWindowMode() {
  if (gtk_widget_get_visible(window)) {
    gtk_window_iconify(GTK_WINDOW(window));
  } else {
    gtk_window_deiconify(GTK_WINDOW(window));
  }
}

/**
 * 好友树(paltree)中是否已经包含此IP地址的好友信息数据.
 * @param ipv4 ipv4
 * @return 是否包含
 */
bool MainWindow::PaltreeContainItem(in_addr ipv4) {
  GtkTreeModel *model;
  GtkTreeIter iter;

  auto pal = app->getCoreThread()->GetPal(ipv4);
  if(!pal) return false;

  auto groupInfo = app->getCoreThread()->GetPalRegularItem(pal.get());
  if(!groupInfo) return false;

  model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "regular-paltree-model"));
  return GroupGetPaltreeItem(model, &iter, groupInfo);
}

/**
 * 更新此IP地址好友在好友树(paltree)中的信息数据.
 * @param ipv4 ipv4
 */
void MainWindow::UpdateItemToPaltree(in_addr ipv4) {
  GtkTreeModel *model;
  GtkTreeIter parent, iter;
  GroupInfo *pgrpinf;

  auto ppal = app->getCoreThread()->GetPal(ipv4);
  if(!ppal) return;
  auto pal = ppal.get();

  auto grpinf = app->getCoreThread()->GetPalRegularItem(pal);
  if(!grpinf) return;

  const char* font = progdt->font;

  /* 更新常规模式树 */
  model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "regular-paltree-model"));
  GroupGetPaltreeItem(model, &iter, grpinf);
  groupInfo2PalTreeModel(grpinf, model, &iter, font);
  /* 更新网段模式树 */
  model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "segment-paltree-model"));
  pgrpinf = g_cthrd->GetPalSegmentItem(pal);
  GroupGetPaltreeItem(model, &iter, pgrpinf);
  GroupGetPaltreeItemWithParent(model, &iter, grpinf);
  groupInfo2PalTreeModel(grpinf, model, &iter, font);
  /* 更新分组模式树 */
  model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "group-paltree-model"));
  pgrpinf = g_cthrd->GetPalGroupItem(pal);
  GroupGetPrevPaltreeItem(model, &iter, grpinf);
  gtk_tree_model_iter_parent(model, &parent, &iter);
  if (gtk_tree_model_iter_n_children(model, &parent) == 1)
    gtk_tree_store_remove(GTK_TREE_STORE(model), &parent);
  else
    gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
  if (!GroupGetPaltreeItem(model, &parent, pgrpinf)) {
    gtk_tree_store_append(GTK_TREE_STORE(model), &parent, NULL);
    palTreeModelFillFromGroupInfo(model, &parent, pgrpinf, progdt->font);
  }
  gtk_tree_store_append(GTK_TREE_STORE(model), &iter, &parent);
  palTreeModelFillFromGroupInfo(model, &iter, grpinf, progdt->font);
  palTreeModelFillFromGroupInfo(model, &parent, pgrpinf, progdt->font);
  /* 更新广播模式树 */
  model =
      GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "broadcast-paltree-model"));
  pgrpinf = g_cthrd->GetPalBroadcastItem(pal);
  GroupGetPaltreeItem(model, &iter, pgrpinf);
  GroupGetPaltreeItemWithParent(model, &iter, grpinf);
  groupInfo2PalTreeModel(grpinf, model, &iter, font);
}

/**
 * 附加此IP地址的好友到好友树(paltree).
 * @param ipv4 ipv4
 */
void MainWindow::AttachItemToPaltree(in_addr ipv4) {
  GtkTreeModel *model;
  GtkTreeIter parent, iter;
  GroupInfo *pgrpinf;

  auto ppal = app->getCoreThread()->GetPal(ipv4);
  if(!ppal) return;
  auto pal = ppal.get();

  auto grpinf = app->getCoreThread()->GetPalRegularItem(pal);
  if(!grpinf) return;

  const char* font = progdt->font;

  /* 添加到常规模式树 */
  model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "regular-paltree-model"));
  gtk_tree_store_append(GTK_TREE_STORE(model), &iter, NULL);
  palTreeModelFillFromGroupInfo(model, &iter, grpinf, progdt->font);
  /* 添加到网段模式树 */
  model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "segment-paltree-model"));
  pgrpinf = g_cthrd->GetPalSegmentItem(pal);
  if (!GroupGetPaltreeItem(model, &parent, pgrpinf)) {
    gtk_tree_store_append(GTK_TREE_STORE(model), &parent, NULL);
    palTreeModelFillFromGroupInfo(model, &parent, pgrpinf, progdt->font);
  }
  gtk_tree_store_append(GTK_TREE_STORE(model), &iter, &parent);
  palTreeModelFillFromGroupInfo(model, &iter, grpinf, progdt->font);
  groupInfo2PalTreeModel(pgrpinf, model, &parent, font);
  /* 添加到分组模式树 */
  model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "group-paltree-model"));
  pgrpinf = g_cthrd->GetPalGroupItem(pal);
  if (!GroupGetPaltreeItem(model, &parent, pgrpinf)) {
    gtk_tree_store_append(GTK_TREE_STORE(model), &parent, NULL);
    palTreeModelFillFromGroupInfo(model, &parent, pgrpinf, progdt->font);
  }
  gtk_tree_store_append(GTK_TREE_STORE(model), &iter, &parent);
  palTreeModelFillFromGroupInfo(model, &iter, grpinf, progdt->font);
  groupInfo2PalTreeModel(pgrpinf, model, &parent, font);
  /* 添加到广播模式树 */
  model =
      GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "broadcast-paltree-model"));
  pgrpinf = g_cthrd->GetPalBroadcastItem(pal);
  if (!GroupGetPaltreeItem(model, &parent, pgrpinf)) {
    gtk_tree_store_append(GTK_TREE_STORE(model), &parent, NULL);
    palTreeModelFillFromGroupInfo(model, &parent, pgrpinf, progdt->font);
  }
  gtk_tree_store_append(GTK_TREE_STORE(model), &iter, &parent);
  palTreeModelFillFromGroupInfo(model, &iter, grpinf, progdt->font);
  groupInfo2PalTreeModel(pgrpinf, model, &parent, font);
}

/**
 * 从好友树(paltree)中删除此IP地址的好友.
 * @param ipv4 ipv4
 */
void MainWindow::DelItemFromPaltree(in_addr ipv4) {
  GtkTreeModel *model;
  GtkTreeIter parent, iter;
  GroupInfo *pgrpinf;

  auto ppal = app->getCoreThread()->GetPal(ipv4);
  if(!ppal) return;
  auto pal = ppal.get();

  auto grpinf = app->getCoreThread()->GetPalRegularItem(pal);
  if(!grpinf) return;

  /* 从常规模式树移除 */
  model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "regular-paltree-model"));
  GroupGetPaltreeItem(model, &iter, grpinf);
  gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
  /* 从网段模式树移除 */
  model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "segment-paltree-model"));
  pgrpinf = g_cthrd->GetPalSegmentItem(pal);
  GroupGetPaltreeItem(model, &parent, pgrpinf);
  if (g_slist_length(pgrpinf->member) != 1) {
    iter = parent;
    GroupGetPaltreeItemWithParent(model, &iter, grpinf);
    gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
    groupInfo2PalTreeModel(pgrpinf, model, &parent, progdt->font);
  } else
    gtk_tree_store_remove(GTK_TREE_STORE(model), &parent);
  /* 从分组模式树移除 */
  model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "group-paltree-model"));
  pgrpinf = g_cthrd->GetPalGroupItem(pal);
  GroupGetPaltreeItem(model, &parent, pgrpinf);
  if (g_slist_length(pgrpinf->member) != 1) {
    iter = parent;
    GroupGetPaltreeItemWithParent(model, &iter, grpinf);
    gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
    groupInfo2PalTreeModel(pgrpinf, model, &parent, progdt->font);
  } else
    gtk_tree_store_remove(GTK_TREE_STORE(model), &parent);
  /* 从广播模式树移除 */
  model =
      GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "broadcast-paltree-model"));
  pgrpinf = g_cthrd->GetPalBroadcastItem(pal);
  GroupGetPaltreeItem(model, &parent, pgrpinf);
  if (g_slist_length(pgrpinf->member) != 1) {
    iter = parent;
    GroupGetPaltreeItemWithParent(model, &iter, grpinf);
    gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
    groupInfo2PalTreeModel(pgrpinf, model, &parent, progdt->font);
  } else
    gtk_tree_store_remove(GTK_TREE_STORE(model), &parent);
}

/**
 * 从好友树(paltree)中删除所有好友数据.
 */
void MainWindow::ClearAllItemFromPaltree() {
  GtkTreeModel *model;

  model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "regular-paltree-model"));
  gtk_tree_store_clear(GTK_TREE_STORE(model));
  model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "segment-paltree-model"));
  gtk_tree_store_clear(GTK_TREE_STORE(model));
  model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "group-paltree-model"));
  gtk_tree_store_clear(GTK_TREE_STORE(model));
  model =
      GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "broadcast-paltree-model"));
  gtk_tree_store_clear(GTK_TREE_STORE(model));
}

/**
 * 让指定的项在好友树(paltree)中闪烁.
 * @param grpinf class GroupInfo
 * @param blinking 是否继续闪烁
 * @note 如果我猜得没错，调用此函数的环境一定已经对(grpinf)加锁了
 */
void MainWindow::MakeItemBlinking(GroupInfo *grpinf, bool blinking) {
  GtkTreeModel *model;
  GtkTreeIter iter;
  GroupInfo *pgrpinf;

  /* 成员为空表明此项已经不存在model中，也就没必要再处理它了 */
  if (!grpinf->member) return;

  /* 闪烁项 */
  switch (grpinf->type) {
    case GROUP_BELONG_TYPE_REGULAR:
      /* 闪烁常规模式树 */
      model =
          GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "regular-paltree-model"));
      GroupGetPaltreeItem(model, &iter, grpinf);
      BlinkGroupItemToPaltree(model, &iter, blinking);
      /* 闪烁网段模式树 */
      model =
          GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "segment-paltree-model"));
      pgrpinf = g_cthrd->GetPalSegmentItem((PalInfo *)grpinf->member->data);
      GroupGetPaltreeItem(model, &iter, pgrpinf);
      GroupGetPaltreeItemWithParent(model, &iter, grpinf);
      BlinkGroupItemToPaltree(model, &iter, blinking);
      /* 闪烁分组模式树 */
      model =
          GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "group-paltree-model"));
      pgrpinf = g_cthrd->GetPalGroupItem((PalInfo *)grpinf->member->data);
      GroupGetPaltreeItem(model, &iter, pgrpinf);
      GroupGetPaltreeItemWithParent(model, &iter, grpinf);
      BlinkGroupItemToPaltree(model, &iter, blinking);
      /* 闪烁广播模式树 */
      model = GTK_TREE_MODEL(
          g_datalist_get_data(&mdlset, "broadcast-paltree-model"));
      pgrpinf = g_cthrd->GetPalBroadcastItem((PalInfo *)grpinf->member->data);
      GroupGetPaltreeItem(model, &iter, pgrpinf);
      GroupGetPaltreeItemWithParent(model, &iter, grpinf);
      BlinkGroupItemToPaltree(model, &iter, blinking);
      break;
    case GROUP_BELONG_TYPE_SEGMENT:
      /* 闪烁网段模式树 */
      model =
          GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "segment-paltree-model"));
      GroupGetPaltreeItem(model, &iter, grpinf);
      BlinkGroupItemToPaltree(model, &iter, blinking);
      break;
    case GROUP_BELONG_TYPE_GROUP:
      /* 闪烁分组模式树 */
      model =
          GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "group-paltree-model"));
      GroupGetPaltreeItem(model, &iter, grpinf);
      BlinkGroupItemToPaltree(model, &iter, blinking);
      break;
    case GROUP_BELONG_TYPE_BROADCAST:
      /* 闪烁广播模式树 */
      model = GTK_TREE_MODEL(
          g_datalist_get_data(&mdlset, "broadcast-paltree-model"));
      GroupGetPaltreeItem(model, &iter, grpinf);
      BlinkGroupItemToPaltree(model, &iter, blinking);
      break;
    default:
      break;
  }
}

/**
 * 打开文件传输窗口.
 */
void MainWindow::OpenTransWindow() {
  if(transWindow == nullptr) {
    transWindow = GTK_WIDGET(trans_window_new(app, GTK_WINDOW(window)));
    gtk_widget_show_all(transWindow);
    gtk_widget_hide(transWindow);
    g_signal_connect_swapped(transWindow, "delete-event", G_CALLBACK(onTransWindowDelete), this);
  }
  gtk_window_present(GTK_WINDOW(transWindow));
}

void MainWindow::UpdateItemToTransTree(const TransFileModel& para) {
  GtkTreeModel *model;
  GtkTreeIter iter;
  gpointer data;

  /* 查询项所在位置，若不存在则自动加入 */
  data = NULL;
  model = GTK_TREE_MODEL(g_object_get_data(G_OBJECT(window), "trans-model"));
  bool found = false;
  if (gtk_tree_model_get_iter_first(model, &iter)) {
    do {
      gtk_tree_model_get(model, &iter, TransModelColumn::PARA, &data, -1);
      if (gpointer(&para) == data) {
        found = true;
        break;
      }
    } while (gtk_tree_model_iter_next(model, &iter));
  }
  if (!found) {
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, TransModelColumn::PARA, &para, -1);
  }

  /* 重设数据 */
  transModelFillFromTransFileModel(model, &iter, para);
  g_action_group_activate_action(
      G_ACTION_GROUP(app->getApp()),
      "trans_model.changed",
      nullptr
  );
}

/**
 * 查询此刻是否存在活动的文件传输.
 * @return 活动与否
 */
bool MainWindow::isTransmissionActive() const {
  GtkTreeModel *model;
  GtkTreeIter iter;
  gpointer data;

  data = nullptr;
  model = GTK_TREE_MODEL(g_object_get_data(G_OBJECT(window), "trans-model"));
  if (gtk_tree_model_get_iter_first(model, &iter)) {
    do {
      gtk_tree_model_get(model, &iter, TransModelColumn::PARA, &data, -1);
      if (data) break;
    } while (gtk_tree_model_iter_next(model, &iter));
  }

  return !!data;
}

/**
 * 初始化底层数据.
 */
void MainWindow::InitSublayer() {
  GtkTreeModel *model;

  g_datalist_init(&widset);
  g_datalist_init(&mdlset);

  accel = gtk_accel_group_new();
  CHECK_EQ(timerid, 0);
  timerid = g_timeout_add(1000, GSourceFunc(UpdateUI), this);

  model = palTreeModelNew();
  g_datalist_set_data_full(&mdlset, "regular-paltree-model", model,
                           GDestroyNotify(g_object_unref));
  tmdllist = g_list_append(tmdllist, model);
  model = palTreeModelNew();
  g_datalist_set_data_full(&mdlset, "segment-paltree-model", model,
                           GDestroyNotify(g_object_unref));
  tmdllist = g_list_append(tmdllist, model);
  model = palTreeModelNew();
  g_datalist_set_data_full(&mdlset, "group-paltree-model", model,
                           GDestroyNotify(g_object_unref));
  tmdllist = g_list_append(tmdllist, model);
  model = palTreeModelNew();
  g_datalist_set_data_full(&mdlset, "broadcast-paltree-model", model,
                           GDestroyNotify(g_object_unref));
  tmdllist = g_list_append(tmdllist, model);
  model = CreatePallistModel();
  g_datalist_set_data_full(&mdlset, "pallist-model", model,
                           GDestroyNotify(g_object_unref));
  InitThemeSublayerData();
}

/**
 * 清空底层数据.
 */
void MainWindow::ClearSublayer() {
  g_datalist_clear(&widset);
  g_datalist_clear(&mdlset);
  g_list_free(tmdllist);
  if (accel) g_object_unref(accel);
  if (timerid > 0) g_source_remove(timerid);
}

/**
 * 创建主窗口.
 * @return 主窗口
 */
GtkWidget *MainWindow::CreateMainWindow() {
  GdkGeometry geometry = {50,
                          200,
                          G_MAXINT,
                          G_MAXINT,
                          0,
                          0,
                          2,
                          5,
                          0.0,
                          0.0,
                          GDK_GRAVITY_NORTH_WEST};
  GdkWindowHints hints = GdkWindowHints(
      GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE | GDK_HINT_BASE_SIZE |
      /*GDK_HINT_RESIZE_INC |*/ GDK_HINT_WIN_GRAVITY | GDK_HINT_USER_POS |
      GDK_HINT_USER_SIZE);
  window = gtk_application_window_new(app->getApp());
  gtk_window_set_icon_name(GTK_WINDOW(window), "iptux");
  if(config->GetString("bind_ip").empty()) {
    gtk_window_set_title(GTK_WINDOW(window), _("iptux"));
  } else {
    gtk_window_set_title(GTK_WINDOW(window), stringFormat("%s - %s", _("iptux"), config->GetString("bind_ip").c_str()).c_str());
  }
  gtk_window_set_default_size(GTK_WINDOW(window), windowConfig.GetWidth(),
                              windowConfig.GetHeight());
  gtk_window_set_geometry_hints(GTK_WINDOW(window), window, &geometry, hints);
  gtk_window_set_default_icon_name("iptux");
  gtk_window_add_accel_group(GTK_WINDOW(window), accel);

  g_signal_connect_swapped(window, "delete-event", G_CALLBACK(onDeleteEvent),
                           this);
  g_signal_connect(window, "configure-event", G_CALLBACK(MWinConfigureEvent),
                   this);
  return window;
}


/**
 * 创建所有区域.
 * @return 主窗体
 */
GtkWidget *MainWindow::CreateAllArea() {
  GtkWidget *box, *paned;

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  gtk_box_pack_start(GTK_BOX(box), CreateToolBar(), FALSE, FALSE, 0);

  paned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
  g_object_set_data(G_OBJECT(paned), "position-name",
                    (gpointer) "mwin-main-paned-divide");
  gtk_paned_set_position(GTK_PANED(paned),
                         config->GetInt("mwin_main_paned_divide", 210));
  gtk_container_set_border_width(GTK_CONTAINER(paned), 4);
  gtk_box_pack_start(GTK_BOX(box), paned, TRUE, TRUE, 0);
  g_signal_connect(paned, "notify::position", G_CALLBACK(PanedDivideChanged),
                   this);
  gtk_paned_pack1(GTK_PANED(paned), CreatePaltreeArea(), TRUE, TRUE);
  gtk_paned_pack2(GTK_PANED(paned), CreatePallistArea(), FALSE, TRUE);

  return box;
}

/**
 * 创建工具条.
 * @return 工具条
 */
GtkWidget *MainWindow::CreateToolBar() {
  GtkWidget *toolbar;
  GtkToolItem *toolitem;
  GtkWidget *widget;

  toolbar = gtk_toolbar_new();
  g_object_set(toolbar, "icon-size", 1, NULL);
  gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);

  toolitem = gtk_tool_button_new(
    gtk_image_new_from_icon_name("go-previous-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR),
    "Go previous"
  );
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
  g_signal_connect_swapped(toolitem, "clicked", G_CALLBACK(GoPrevTreeModel),
                           this);

  toolitem = gtk_tool_item_new();
  gtk_tool_item_set_expand(toolitem, TRUE);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
  widget = gtk_label_new(_("Pals Online: 0"));
  gtk_container_add(GTK_CONTAINER(toolitem), widget);
  g_datalist_set_data(&widset, "online-label-widget", widget);

  toolitem = gtk_tool_button_new(
    gtk_image_new_from_icon_name("go-next-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR),
    "Go next"
  );
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
  g_signal_connect_swapped(toolitem, "clicked", G_CALLBACK(GoNextTreeModel),
                           this);

  return toolbar;
}

/**
 * 创建好友树区域.
 * @return 主窗体
 */
GtkWidget *MainWindow::CreatePaltreeArea() {
  GtkWidget *sw;
  GtkWidget *widget;
  GtkTreeModel *model;

  sw = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                      GTK_SHADOW_ETCHED_IN);

  model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "regular-paltree-model"));
  widget = CreatePaltreeTree(model);
  g_object_set_data(G_OBJECT(widget), "paltree-model", model);
  gtk_container_add(GTK_CONTAINER(sw), widget);
  g_datalist_set_data(&widset, "paltree-treeview-widget", widget);

  return sw;
}

/**
 * 创建好友清单区域.
 * @return 主窗体
 */
GtkWidget *MainWindow::CreatePallistArea() {
  GtkWidget *box, *hbox;
  GtkWidget *sw, *button, *widget;
  GtkTreeModel *model;

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  g_datalist_set_data(&widset, "pallist-box-widget", box);

  /* 创建好友清单部分 */
  sw = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                      GTK_SHADOW_ETCHED_IN);
  gtk_box_pack_start(GTK_BOX(box), sw, TRUE, TRUE, 0);
  model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "pallist-model"));
  widget = CreatePallistTree(model);
  gtk_container_add(GTK_CONTAINER(sw), widget);
  g_datalist_set_data(&widset, "pallist-treeview-widget", widget);

  /* 创建接受搜索输入部分 */
  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
  /* 关闭按钮 */
  button = gtk_button_new();
  widget = gtk_image_new_from_icon_name("window-close-symbolic", GTK_ICON_SIZE_BUTTON);
  gtk_button_set_image(GTK_BUTTON(button), widget);
  g_object_set(button, "relief", GTK_RELIEF_NONE, NULL);
  gtk_widget_set_size_request(button, -1, 1);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
  g_signal_connect_swapped(button, "clicked", G_CALLBACK(HidePallistArea),
                           &widset);
  /* 输入框 */
  widget = gtk_entry_new();
  gtk_entry_set_activates_default(GTK_ENTRY(widget), TRUE);
  gtk_entry_set_icon_from_icon_name(GTK_ENTRY(widget), GTK_ENTRY_ICON_SECONDARY,
                                "edit-find-symbolic");
  gtk_widget_add_events(widget, GDK_KEY_PRESS_MASK);
  g_object_set(widget, "has-tooltip", TRUE, NULL);
  gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
  g_signal_connect(widget, "query-tooltip", G_CALLBACK(entry_query_tooltip),
                   _("Search Pals"));
  g_signal_connect(widget, "key-press-event", G_CALLBACK(ClearPallistEntry),
                   NULL);
  g_signal_connect(widget, "changed", G_CALLBACK(PallistEntryChanged), &widset);
  g_datalist_set_data(&widset, "pallist-entry-widget", widget);

  return box;
}

/**
 * 好友清单(pallist)底层数据结构.
 * 7,0 icon,1 name,2 group,3 ipstr,4 user,5 host,6 data \n
 * 好友头像;好友昵称;好友群组;IP地址串;好友用户;好友主机;好友数据 \n
 * @return pallist-model
 * @note 鉴于好友清单(pallist)常年保持隐藏状态，所以请不要有事没事就往
 * 此model中填充数据，好友清单也无须与好友最新状态保持同步.
 */
GtkTreeModel *MainWindow::CreatePallistModel() {
  GtkListStore *model;

  model = gtk_list_store_new(7, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING,
                             G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                             G_TYPE_POINTER);

  return GTK_TREE_MODEL(model);
}

/**
 * 创建好友树(paltree).
 * @param model paltree-model
 * @return 好友树
 */
GtkWidget *MainWindow::CreatePaltreeTree(GtkTreeModel *model) {
  GtkWidget *view;
  GtkTreeSelection *selection;
  GtkTreeViewColumn *column;
  GtkCellRenderer *cell;

  view = gtk_tree_view_new_with_model(model);
  gtk_tree_view_set_level_indentation(GTK_TREE_VIEW(view), 10);
  gtk_tree_view_set_show_expanders(GTK_TREE_VIEW(view), FALSE);
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);
  widget_enable_dnd_uri(view);
  g_object_set(view, "has-tooltip", TRUE, NULL);
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
  gtk_tree_selection_set_mode(GTK_TREE_SELECTION(selection),
                              GTK_SELECTION_NONE);

  column = gtk_tree_view_column_new();
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
  g_object_set_data(G_OBJECT(view), "info-column", column);
  /* 展开器区域 */
  cell = gtk_cell_renderer_pixbuf_new();
  g_object_set(cell, "follow-state", TRUE, NULL);
  gtk_tree_view_column_pack_start(column, cell, FALSE);
  gtk_tree_view_column_set_attributes(GTK_TREE_VIEW_COLUMN(column), cell,
                                      "pixbuf", PalTreeModelColumn ::CLOSED_EXPANDER,
                                      "pixbuf-expander-closed", PalTreeModelColumn ::CLOSED_EXPANDER,
                                      "pixbuf-expander-open", PalTreeModelColumn ::OPEN_EXPANDER,
                                      NULL);
  g_object_set_data(G_OBJECT(column), "expander-cell", cell);
  /* 群组信息区域 */
  cell = gtk_cell_renderer_text_new();
  g_object_set(cell, "xalign", 0.0, "wrap-mode", PANGO_WRAP_WORD, NULL);
  gtk_tree_view_column_pack_start(column, cell, FALSE);
  gtk_tree_view_column_set_attributes(GTK_TREE_VIEW_COLUMN(column), cell,
                                      "text", PalTreeModelColumn ::INFO,
                                      "attributes", PalTreeModelColumn ::STYLE,
                                      "foreground-rgba", PalTreeModelColumn ::COLOR,
                                      NULL);
  /* 扩展信息区域 */
  cell = gtk_cell_renderer_text_new();
  g_object_set(cell, "xalign", 0.0, "wrap-mode", PANGO_WRAP_WORD, NULL);
  gtk_tree_view_column_pack_start(column, cell, FALSE);
  gtk_tree_view_column_set_attributes(GTK_TREE_VIEW_COLUMN(column), cell,
                                      "text", PalTreeModelColumn ::EXTRAS,
                                      "attributes", PalTreeModelColumn ::STYLE,
                                      "foreground-rgba", PalTreeModelColumn ::COLOR,
                                      NULL);

  /* 连接信号 */
  g_signal_connect(view, "query-tooltip", G_CALLBACK(PaltreeQueryTooltip),
                   this);
  g_signal_connect(view, "row-activated", G_CALLBACK(onPaltreeItemActivated),
                   this);
  g_signal_connect(view, "drag-data-received",
                   G_CALLBACK(PaltreeDragDataReceived), this);
  g_signal_connect(view, "button-press-event", G_CALLBACK(PaltreePopupMenu),
                   NULL);
  g_signal_connect(view, "button-release-event",
                   G_CALLBACK(PaltreeChangeStatus), NULL);

  return view;
}

/**
 * 创建好友清单(pallist).
 * @param model pallist-model
 * @return 好友清单
 */
GtkWidget *MainWindow::CreatePallistTree(GtkTreeModel *model) {
  GtkWidget *view;
  GtkTreeViewColumn *column;
  GtkCellRenderer *cell;
  GtkTreeSelection *selection;

  view = gtk_tree_view_new_with_model(model);
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), TRUE);
  gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(view), TRUE);
  widget_enable_dnd_uri(view);
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
  gtk_tree_selection_set_mode(GTK_TREE_SELECTION(selection),
                              GTK_SELECTION_NONE);

  column = gtk_tree_view_column_new();
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_column_set_title(column, _("Nickname"));
  cell = gtk_cell_renderer_pixbuf_new();
  g_object_set(cell, "follow-state", TRUE, NULL);
  gtk_tree_view_column_pack_start(column, cell, FALSE);
  gtk_tree_view_column_set_attributes(column, cell, "pixbuf", 0, NULL);
  cell = gtk_cell_renderer_text_new();
  gtk_tree_view_column_pack_start(column, cell, FALSE);
  gtk_tree_view_column_set_attributes(column, cell, "text", 1, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Group"), cell, "text", 2,
                                                    NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("IPv4"), cell, "text", 3,
                                                    NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("User"), cell, "text", 4,
                                                    NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Host"), cell, "text", 5,
                                                    NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  g_signal_connect(view, "row-activated", G_CALLBACK(PallistItemActivated),
                   this);
  g_signal_connect(view, "drag-data-received",
                   G_CALLBACK(PallistDragDataReceived), this);

  return view;
}

/**
 * 获取项(grpinf)在数据集(model)中的当前位置.
 * @param model model
 * @param iter 位置由此返回
 * @param grpinf class GroupInfo
 * @return 是否查找成功
 */
bool MainWindow::GroupGetPrevPaltreeItem(GtkTreeModel *model, GtkTreeIter *iter,
                                         GroupInfo *grpinf) {
  GroupInfo *pgrpinf;
  GtkTreeIter parent;

  if (!gtk_tree_model_get_iter_first(model, &parent)) return false;
  do {
    gtk_tree_model_get(model, &parent, 6, &pgrpinf, -1);
    if (pgrpinf->grpid == grpinf->grpid) {
      *iter = parent;
      break;
    }
    if (!gtk_tree_model_iter_children(model, iter, &parent)) continue;
    do {
      gtk_tree_model_get(model, iter, 6, &pgrpinf, -1);
      if (pgrpinf->grpid == grpinf->grpid) break;
    } while (gtk_tree_model_iter_next(model, iter));
    if (pgrpinf->grpid == grpinf->grpid) break;
  } while (gtk_tree_model_iter_next(model, &parent));

  return (pgrpinf->grpid == grpinf->grpid);
}

/**
 * 获取项(grpinf)在数据集(model)中的位置.
 * @param model model
 * @param iter 位置由此返回
 * @param grpinf class GroupInfo
 * @return 是否查找成功
 */
bool MainWindow::GroupGetPaltreeItem(GtkTreeModel *model, GtkTreeIter *iter,
                                     GroupInfo *grpinf) {
  if (!gtk_tree_model_get_iter_first(model, iter)) return false;
  do {
    GroupInfo *pgrpinf;
    gtk_tree_model_get(model, iter, 6, &pgrpinf, -1);
    if(pgrpinf == nullptr) {
      LOG_WARN("don't have pgrpinf in this model and iter: %p, %p", model, iter);
      continue;
    }
    if (pgrpinf->grpid == grpinf->grpid) {
      return true;
    }
  } while (gtk_tree_model_iter_next(model, iter));
  return false;
}

/**
 * 获取项(grpinf)在数据集(model)中的位置.
 * @param model model
 * @param iter 父节点位置/位置由此返回
 * @param grpinf class GroupInfo
 * @return 是否查找成功
 */
bool MainWindow::GroupGetPaltreeItemWithParent(GtkTreeModel *model,
                                               GtkTreeIter *iter,
                                               GroupInfo *grpinf) {
  GtkTreeIter parent;
  GroupInfo *pgrpinf;

  parent = *iter;
  if (!gtk_tree_model_iter_children(model, iter, &parent)) return false;
  do {
    gtk_tree_model_get(model, iter, 6, &pgrpinf, -1);
    if (pgrpinf->grpid == grpinf->grpid) break;
  } while (gtk_tree_model_iter_next(model, iter));

  return (pgrpinf->grpid == grpinf->grpid);
}

/**
 * 闪烁指定项.
 * @param model model
 * @param iter iter
 * @param blinking 是否继续闪烁
 */
void MainWindow::BlinkGroupItemToPaltree(GtkTreeModel *model, GtkTreeIter *iter,
                                         bool blinking) {
  static GdkRGBA color1 = {0.3216, 0.7216, 0.2196, 0.0},
                 color2 = {0.0, 0.0, 1.0, 0.0};
  GdkRGBA *color;

  if (blinking) {
    gtk_tree_model_get(model, iter, 5, &color, -1);
    if(gdk_rgba_equal(color, &color1)) {
      gtk_tree_store_set(GTK_TREE_STORE(model), iter, 5, &color2, -1);
    } else {
      gtk_tree_store_set(GTK_TREE_STORE(model), iter, 5, &color1, -1);
    }
  } else
    gtk_tree_store_set(GTK_TREE_STORE(model), iter, 5, &color1, -1);
}

/**
 * @param pal class PalInfo
 * @return use g_free to free the return value
 */
gchar* palInfo2HintMarkup(const PalInfo *pal) {
  char ipstr[INET_ADDRSTRLEN];
  gchar *version = g_markup_printf_escaped(_("Version: %s"), pal->version);
  gchar *nickname;
  if (pal->group && *pal->group != '\0') {
    nickname = g_markup_printf_escaped(_("Nickname: %s@%s"), pal->name, pal->group);
  } else {
    nickname = g_markup_printf_escaped(_("Nickname: %s"), pal->name);
  }
  gchar *user = g_markup_printf_escaped(_("User: %s"), pal->user);
  gchar *host = g_markup_printf_escaped(_("Host: %s"), pal->host);
  gchar *address;
  inet_ntop(AF_INET, &pal->ipv4, ipstr, INET_ADDRSTRLEN);
  if (pal->segdes && *pal->segdes != '\0') {
    address = g_markup_printf_escaped(_("Address: %s(%s)"), pal->segdes, ipstr);
  } else {
    address = g_markup_printf_escaped(_("Address: %s"), ipstr);
  }
  gchar *compatibility;
  if (!pal->isCompatible()) {
    compatibility = g_markup_escape_text(_("Compatibility: Microsoft"), -1);
  } else {
    compatibility = g_markup_escape_text(_("Compatibility: GNU/Linux"), -1);
  }
  gchar *coding = g_markup_printf_escaped(_("System coding: %s"), pal->encode);
  gchar *signature1 = nullptr;
  gchar *signature2 = nullptr;
  if (pal->sign && *pal->sign != '\0') {
    signature1 = g_markup_printf_escaped("%s", _("Signature:"));
    signature2 = g_markup_escape_text(pal->sign, -1);
  }

  gchar* result;
  if(signature1) {
    result = g_strdup_printf("%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n<span foreground=\"#00FF00\" font_style=\"italic\" size=\"smaller\">%s</span>",
                        version, nickname, user, host, address, compatibility, coding,
                        signature1, signature2);
  } else {
    result = g_strdup_printf("%s\n%s\n%s\n%s\n%s\n%s\n%s",
                             version, nickname, user, host, address, compatibility, coding);
  }
  g_free(version);
  g_free(nickname);
  g_free(user);
  g_free(host);
  g_free(address);
  g_free(coding);
  g_free(compatibility);
  g_free(signature1);
  g_free(signature2);
  return result;
}

/**
 * 更新UI.
 * @param mwin 主窗口类
 * @return Gtk+库所需
 */
gboolean MainWindow::UpdateUI(MainWindow *mwin) {
  static uint32_t sumonline = 0;  //避免每次都作一次设置
  GtkWidget *widget;
  uint32_t sum;

  /* 统计当前在线人数 */
  sum = g_cthrd->GetOnlineCount();

  /* 更新UI */
  if (sumonline != sum) {
    auto label = stringFormat(_("Pals Online: %" PRIu32), sum);
    widget =
        GTK_WIDGET(g_datalist_get_data(&mwin->widset, "online-label-widget"));
    gtk_label_set_text(GTK_LABEL(widget), label.c_str());
    sumonline = sum;
  }

  return TRUE;
}

/**
 * 转到上一类结构树.
 * @param mwin 主窗口类
 */
void MainWindow::GoPrevTreeModel(MainWindow *mwin) {
  GtkWidget *widget;
  GtkTreeModel *model;
  GtkTreeViewColumn *column;
  GList *tlist;

  widget =
      GTK_WIDGET(g_datalist_get_data(&mwin->widset, "paltree-treeview-widget"));
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  if ((tlist = g_list_find(mwin->tmdllist, model))) {
    if (tlist->prev)
      model = GTK_TREE_MODEL(tlist->prev->data);
    else
      model = GTK_TREE_MODEL(g_list_last(mwin->tmdllist)->data);
    gtk_tree_view_set_model(GTK_TREE_VIEW(widget), model);
  }
  column =
      GTK_TREE_VIEW_COLUMN(g_object_get_data(G_OBJECT(widget), "info-column"));
  gtk_tree_view_column_queue_resize(column);
}

/**
 * 转到下一类结构树.
 * @param mwin 主窗口类
 */
void MainWindow::GoNextTreeModel(MainWindow *mwin) {
  GtkWidget *widget;
  GtkTreeModel *model;
  GtkTreeViewColumn *column;
  GList *tlist;

  widget =
      GTK_WIDGET(g_datalist_get_data(&mwin->widset, "paltree-treeview-widget"));
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  if ((tlist = g_list_find(mwin->tmdllist, model))) {
    if (tlist->next)
      model = GTK_TREE_MODEL(tlist->next->data);
    else
      model = GTK_TREE_MODEL(mwin->tmdllist->data);
    gtk_tree_view_set_model(GTK_TREE_VIEW(widget), model);
  }
  column =
      GTK_TREE_VIEW_COLUMN(g_object_get_data(G_OBJECT(widget), "info-column"));
  gtk_tree_view_column_queue_resize(column);
}

/**
 * 更新好友树.
 * @param mwin 主窗口类
 */
void MainWindow::onRefresh(void*, void*, MainWindow& self) {
  auto mwin = &self;

  pthread_t pid;

  g_cthrd->Lock();
  mwin->ClearAllItemFromPaltree();
  g_cthrd->ClearAllPalFromList();
  g_cthrd->Unlock();

  pthread_create(&pid, NULL, ThreadFunc(UiCoreThread::SendNotifyToAll), g_cthrd);
  pthread_detach(pid);
}

void MainWindow::onDetect(void*, void*, MainWindow& self) {
  DetectPal pal(self.app, self.builder, GTK_WINDOW(self.window));
  pal.run();
}


void MainWindow::onSortBy(GSimpleAction *action, GVariant* value, MainWindow& self) {
  string sortBy = g_variant_get_string(value, nullptr);

  PalTreeModelSortKey key;

  if(sortBy == "nickname") {
    key = PalTreeModelSortKey::NICKNAME;
  } else if(sortBy == "ip") {
    key = PalTreeModelSortKey::IP;
  } else {
    LOG_WARN("unknown sort by: %s", sortBy.c_str());
    return;
  }


  GtkTreeModel *model;

  model = GTK_TREE_MODEL(g_datalist_get_data(&self.mdlset, "regular-paltree-model"));
  palTreeModelSetSortKey(model, key);

  model = GTK_TREE_MODEL(g_datalist_get_data(&self.mdlset, "segment-paltree-model"));
  palTreeModelSetSortKey(model, key);

  model = GTK_TREE_MODEL(g_datalist_get_data(&self.mdlset, "group-paltree-model"));
  palTreeModelSetSortKey(model, key);

  model =
      GTK_TREE_MODEL(g_datalist_get_data(&self.mdlset, "broadcast-paltree-model"));
  palTreeModelSetSortKey(model, key);
  g_simple_action_set_state(action, value);
}


void MainWindow::onSortType(GSimpleAction *action, GVariant *value, MainWindow &self) {
  string sortType = g_variant_get_string(value, nullptr);

  GtkSortType type;

  if(sortType == "ascending") {
    type = GTK_SORT_ASCENDING;
  } else if(sortType == "descending") {
    type = GTK_SORT_DESCENDING;
  } else {
    LOG_WARN("unknown sorttype: %s", sortType.c_str());
    return;
  }

  GtkTreeModel *model;

  model = GTK_TREE_MODEL(g_datalist_get_data(&self.mdlset, "regular-paltree-model"));
  gtk_tree_sortable_set_sort_column_id(
      GTK_TREE_SORTABLE(model), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, type);

  model = GTK_TREE_MODEL(g_datalist_get_data(&self.mdlset, "segment-paltree-model"));
  gtk_tree_sortable_set_sort_column_id(
      GTK_TREE_SORTABLE(model), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, type);

  model = GTK_TREE_MODEL(g_datalist_get_data(&self.mdlset, "group-paltree-model"));
  gtk_tree_sortable_set_sort_column_id(
      GTK_TREE_SORTABLE(model), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, type);

  model =
      GTK_TREE_MODEL(g_datalist_get_data(&self.mdlset, "broadcast-paltree-model"));
  gtk_tree_sortable_set_sort_column_id(
      GTK_TREE_SORTABLE(model), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, type);
  g_simple_action_set_state(action, value);
}

/**
 * 删除好友项.
 * @param grpinf 好友群组信息
 */
void MainWindow::DeletePalItem(GroupInfo *grpinf) {
  PalInfo *pal;

  /* 从UI中移除 */
  if (g_mwin->PaltreeContainItem(inAddrFromUint32(grpinf->grpid))) {
    g_mwin->DelItemFromPaltree(inAddrFromUint32(grpinf->grpid));
  }

  g_cthrd->Lock();
  auto ppal = g_cthrd->GetPal(inAddrFromUint32(grpinf->grpid));
  /* 从数据中心点移除 */
  if (ppal) {
    g_cthrd->DelPalFromList(inAddrFromUint32(grpinf->grpid));
    ppal->setOnline(false);
  }
  /* 加入黑名单 */
  if (!g_cthrd->BlacklistContainItem(inAddrFromUint32(grpinf->grpid))) {
    g_cthrd->AddBlockIp(inAddrFromUint32(grpinf->grpid));
  }
  g_cthrd->Unlock();
}

/**
 * 好友树(paltree)信息提示查询请求.
 * @param treeview the object which received the signal
 * @param x the x coordinate of the cursor position
 * @param y the y coordinate of the cursor position
 * @param keyboard_mode TRUE if the tooltip was triggered using the keyboard
 * @param tooltip a GtkTooltip
 * @return Gtk+库所需
 */
gboolean MainWindow::PaltreeQueryTooltip(GtkWidget *treeview, gint x, gint y,
                                         gboolean keyboard_mode, GtkTooltip *tooltip,
                                         MainWindow *) {
  GtkTreePath *path;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gint bx, by;
  GroupInfo *grpinf;

  gtk_tree_view_convert_widget_to_bin_window_coords(GTK_TREE_VIEW(treeview), x,
                                                    y, &bx, &by);
  if (keyboard_mode || !gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), bx, by,
                                            &path, NULL, NULL, NULL)) {
    return FALSE;
  }

  model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
  gtk_tree_model_get_iter(model, &iter, path);
  gtk_tree_path_free(path);
  gtk_tree_model_get(model, &iter, 6, &grpinf, -1);
  if (grpinf->type != GROUP_BELONG_TYPE_REGULAR) return FALSE;

  char* tooltipMarkup = palInfo2HintMarkup((PalInfo *)grpinf->member->data);
  gtk_tooltip_set_markup(tooltip, tooltipMarkup);
  g_free(tooltipMarkup);

  return TRUE;
}

/**
 * 好友树(paltree)项被激活.
 * @param treeview the object on which the signal is emitted
 * @param path the GtkTreePath for the activated row
 * @param column the GtkTreeViewColumn in which the activation occurred
 */
void MainWindow::onPaltreeItemActivated(GtkWidget *treeview, GtkTreePath *path,
                                        GtkTreeViewColumn *,
                                        MainWindow *self) {
  GtkTreeModel *model;
  GtkTreeIter iter;
  GroupInfo *grpinf;

  /* 获取项关联的群组数据 */
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
  gtk_tree_model_get_iter(model, &iter, path);
  gtk_tree_model_get(model, &iter, 6, &grpinf, -1);
  /* 检查是否需要新建对话框 */
  if (grpinf->dialog) {
    gtk_window_present(GTK_WINDOW(grpinf->dialog));
    return;
  }

  /* 根据需求建立对应的对话框 */
  switch (grpinf->type) {
    case GROUP_BELONG_TYPE_REGULAR:
      DialogPeer::PeerDialogEntry(self->app, grpinf);
      break;
    case GROUP_BELONG_TYPE_SEGMENT:
    case GROUP_BELONG_TYPE_GROUP:
    case GROUP_BELONG_TYPE_BROADCAST:
      DialogGroup::GroupDialogEntry(self->app, grpinf);
    default:
      break;
  }
}

/**
 * 好友树(paltree)弹出操作菜单.
 * @param treeview tree-view
 * @param event event
 * @return Gtk+库所需
 */
gboolean MainWindow::PaltreePopupMenu(
  GtkWidget *treeview,
  GdkEventButton *event)
{
  GtkWidget *menu;
  GtkTreeModel *model;
  GtkTreePath *path;
  GtkTreeIter iter;
  GroupInfo *grpinf;

  /* 检查事件是否可用 */
  if (event->button != GDK_BUTTON_SECONDARY ||
      !gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), (event->x),
                                     (event->y), &path, NULL, NULL, NULL)) {
    return FALSE;
  }

  /* 获取好友群组信息数据 */
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
  gtk_tree_model_get_iter(model, &iter, path);
  gtk_tree_path_free(path);
  gtk_tree_model_get(model, &iter, 6, &grpinf, -1);
  g_mwin->setCurrentGroupInfo(grpinf);
  gtk_widget_show_all(GTK_WIDGET(g_mwin->palPopupMenu));
  gtk_menu_popup_at_pointer(GTK_MENU(g_mwin->palPopupMenu), NULL);
  return TRUE;
}

/**
 * 展开或隐藏某行.
 * @param treeview text-view
 * @param event event
 * @return Gtk+库所需
 */
gboolean MainWindow::PaltreeChangeStatus(GtkWidget *treeview,
                                         GdkEventButton *event) {
  GtkTreeModel *model;
  GtkCellRenderer *cell;
  GtkTreeViewColumn *column;
  GtkTreePath *path;
  GtkTreeIter iter;
  gint cellx, startpos, width;
  GroupInfo *grpinf;

  /* 检查事件的合法性 */
  if (event->button != 1 || !gtk_tree_view_get_path_at_pos(
                                GTK_TREE_VIEW(treeview), (event->x),
                                (event->y), &path, &column, &cellx, NULL))
    return FALSE;

  /* 检查此行是否可展开 */
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
  gtk_tree_model_get_iter(model, &iter, path);
  gtk_tree_model_get(model, &iter, 6, &grpinf, -1);
  if (grpinf->type == GROUP_BELONG_TYPE_REGULAR) {
    gtk_tree_path_free(path);
    return FALSE;
  }

  /* 检查事件所发生的位置是否正确 */
  cell =
      GTK_CELL_RENDERER(g_object_get_data(G_OBJECT(column), "expander-cell"));
  gtk_tree_view_column_cell_get_position(column, cell, &startpos, &width);
  if ((cellx < startpos) || (cellx > startpos + width)) {
    gtk_tree_path_free(path);
    return FALSE;
  }

  /* 展开或隐藏行 */
  if (gtk_tree_view_row_expanded(GTK_TREE_VIEW(treeview), path))
    gtk_tree_view_collapse_row(GTK_TREE_VIEW(treeview), path);
  else
    gtk_tree_view_expand_row(GTK_TREE_VIEW(treeview), path, FALSE);
  gtk_tree_path_free(path);

  return TRUE;
}

/**
 * 好友树(paltree)拖拽事件响应处理函数.
 * @param treeview tree-view
 * @param context the drag context
 * @param x where the drop happened
 * @param y where the drop happened
 * @param data the received data
 * @param info the info that has been registered with the target in the
 * GtkTargetList
 * @param time the timestamp at which the data was received
 */
void MainWindow::PaltreeDragDataReceived(GtkWidget *treeview,
                                         GdkDragContext *, gint x,
                                         gint y, GtkSelectionData *data,
                                         guint , guint ,
                                         MainWindow *self) {
  GtkTreeModel *model;
  GtkTreePath *path;
  GtkTreeIter iter;
  gint bx, by;
  GroupInfo *grpinf;
  SessionAbstract *session;
  GSList *list;

  /* 事件是否可用 */
  gtk_tree_view_convert_widget_to_bin_window_coords(GTK_TREE_VIEW(treeview), x,
                                                    y, &bx, &by);
  if (!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), bx, by, &path,
                                     NULL, NULL, NULL))
    return;

  /* 获取好友群组信息数据 */
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
  gtk_tree_model_get_iter(model, &iter, path);
  gtk_tree_path_free(path);
  gtk_tree_model_get(model, &iter, 6, &grpinf, -1);

  /* 如果好友群组对话框尚未创建，则先创建对话框 */
  if (!(grpinf->dialog)) {
    switch (grpinf->type) {
      case GROUP_BELONG_TYPE_REGULAR:
        DialogPeer::PeerDialogEntry(self->app, grpinf);
        break;
      case GROUP_BELONG_TYPE_SEGMENT:
      case GROUP_BELONG_TYPE_GROUP:
      case GROUP_BELONG_TYPE_BROADCAST:
        DialogGroup::GroupDialogEntry(self->app, grpinf);
      default:
        break;
    }
  } else
    gtk_window_present(GTK_WINDOW(grpinf->dialog));
  /* 获取会话对象，并将数据添加到会话对象 */
  session = (SessionAbstract *)g_object_get_data(G_OBJECT(grpinf->dialog),
                                                 "session-class");
  list = selection_data_get_path(data);  //获取所有文件
  session->AttachEnclosure(list);
  g_slist_foreach(list, GFunc(g_free), NULL);
  g_slist_free(list);
  //        session->ShowEnclosure();
}

/**
 * 显示好友清单区域.
 * @param widset widget set
 */
void MainWindow::onFind(void*, void*, MainWindow&self) {
  GtkWidget *widget;

  widget = GTK_WIDGET(g_datalist_get_data(&self.widset, "pallist-box-widget"));
  gtk_widget_show(widget);
  widget = GTK_WIDGET(g_datalist_get_data(&self.widset, "pallist-entry-widget"));
  gtk_widget_grab_focus(widget);
  PallistEntryChanged(widget, &self.widset);
}

void MainWindow::onDeletePal(void*, void*, MainWindow&self) {
  GroupInfo* groupInfo = CHECK_NOTNULL(self.currentGroupInfo);
  switch(groupInfo->type) {
    case GROUP_BELONG_TYPE_REGULAR:
      self.DeletePalItem(groupInfo);
      break;
    default:
      CHECK(false);
      break;
  }
}

void MainWindow::onPalChangeInfo(void*, void*, MainWindow&self) {
  GroupInfo* groupInfo = CHECK_NOTNULL(self.currentGroupInfo);
  switch(groupInfo->type) {
    case GROUP_BELONG_TYPE_REGULAR:
      RevisePal::ReviseEntry((PalInfo*)(groupInfo->member->data));
      break;
    default:
      CHECK(false);
      break;
  }
}

void MainWindow::onPalSendMessage(void*, void*, MainWindow&self) {
  GroupInfo* groupInfo = CHECK_NOTNULL(self.currentGroupInfo);
  switch(groupInfo->type) {
    case GROUP_BELONG_TYPE_REGULAR:
      DialogPeer::PeerDialogEntry(self.app, groupInfo);
      break;
    case GROUP_BELONG_TYPE_SEGMENT:
    case GROUP_BELONG_TYPE_GROUP:
    case GROUP_BELONG_TYPE_BROADCAST:
      DialogGroup::GroupDialogEntry(self.app, groupInfo);
      break;
    default:
      CHECK(false);
      break;
  }
}
void MainWindow::onPalRequestSharedResources(void*, void*, MainWindow&self) {
  GroupInfo* groupInfo = CHECK_NOTNULL(self.currentGroupInfo);
  switch(groupInfo->type) {
    case GROUP_BELONG_TYPE_REGULAR:
      g_cthrd->SendAskShared(g_cthrd->GetPal(((PalInfo *)groupInfo->member->data)->GetKey()));
      break;
    default:
      CHECK(false);
      break;
  }
}

/**
 * 隐藏好友清单区域.
 * @param widset widget set
 */
void MainWindow::HidePallistArea(GData **widset) {
  GtkWidget *widget;
  GtkTreeModel *model;

  widget = GTK_WIDGET(g_datalist_get_data(widset, "pallist-box-widget"));
  gtk_widget_hide(widget);
  widget = GTK_WIDGET(g_datalist_get_data(widset, "pallist-treeview-widget"));
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  gtk_list_store_clear(GTK_LIST_STORE(model));
  widget = GTK_WIDGET(g_datalist_get_data(widset, "pallist-entry-widget"));
  gtk_editable_delete_text(GTK_EDITABLE(widget), 0, -1);
}

/**
 * 清空好友清单搜索输入框.
 * @param entry entry
 * @param event event
 * @return Gtk+库所需
 */
gboolean MainWindow::ClearPallistEntry(GtkWidget *entry, GdkEventKey *event) {
  if (event->keyval != GDK_KEY_Escape) return FALSE;
  gtk_editable_delete_text(GTK_EDITABLE(entry), 0, -1);
  return TRUE;
}

/**
 * 好友清单搜索输入框内容变更响应处理函数.
 * @param entry entry
 * @param widset widget set
 */
void MainWindow::PallistEntryChanged(GtkWidget *entry, GData **widset) {
  GtkIconTheme *theme;
  GdkPixbuf *pixbuf;
  GtkWidget *treeview;
  GtkTreeModel *model;
  GtkTreeIter iter;
  char ipstr[INET_ADDRSTRLEN], *file;
  const gchar *text;

  /* 获取默认主题 */
  theme = gtk_icon_theme_get_default();
  /* 获取搜索内容 */
  text = gtk_entry_get_text(GTK_ENTRY(entry));
  /* 获取好友清单，并清空 */
  treeview = GTK_WIDGET(g_datalist_get_data(widset, "pallist-treeview-widget"));
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
  gtk_list_store_clear(GTK_LIST_STORE(model));

  /* 将符合条件的好友加入好友清单 */
  for(auto pal: g_cthrd->GetPalList()) {
    inet_ntop(AF_INET, &pal->ipv4, ipstr, INET_ADDRSTRLEN);
    /* Search friends case ignore is better. */
    if (*text == '\0' || strcasestr(pal->name, text) ||
        (pal->group && strcasestr(pal->group, text)) ||
        strcasestr(ipstr, text) || strcasestr(pal->user, text) ||
        strcasestr(pal->host, text)) {
      file = iptux_erase_filename_suffix(pal->iconfile);
      pixbuf = gtk_icon_theme_load_icon(theme, file, MAX_ICONSIZE,
                                        GtkIconLookupFlags(0), NULL);
      g_free(file);
      gtk_list_store_append(GTK_LIST_STORE(model), &iter);
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, pixbuf, 1, pal->name,
                         2, pal->group, 3, ipstr, 4, pal->user, 5, pal->host, 6,
                         pal.get(), -1);
      if (pixbuf) g_object_unref(pixbuf);
    }
  }

  /* 重新调整好友清单UI */
  gtk_tree_view_columns_autosize(GTK_TREE_VIEW(treeview));
}

/**
 * 好友清单(pallist)项被激活.
 * @param treeview the object on which the signal is emitted
 * @param path the GtkTreePath for the activated row
 * @param column the GtkTreeViewColumn in which the activation occurred
 */
void MainWindow::PallistItemActivated(GtkWidget *treeview, GtkTreePath *path,
                                      GtkTreeViewColumn *,
                                      MainWindow *self) {
  GtkTreeModel *model;
  GtkTreeIter iter;
  GroupInfo *grpinf;
  PalInfo *pal;

  model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
  gtk_tree_model_get_iter(model, &iter, path);
  gtk_tree_model_get(model, &iter, 6, &pal, -1);
  if ((grpinf = g_cthrd->GetPalRegularItem(pal))) {
    if (!(grpinf->dialog))
      DialogPeer::PeerDialogEntry(self->app, grpinf);
    else
      gtk_window_present(GTK_WINDOW(grpinf->dialog));
  }
}

/**
 * 好友清单(pallist)拖拽事件响应处理函数.
 * @param treeview tree-view
 * @param context the drag context
 * @param x where the drop happened
 * @param y where the drop happened
 * @param data the received data
 * @param info the info that has been registered with the target in the
 * GtkTargetList
 * @param time the timestamp at which the data was received
 */
void MainWindow::PallistDragDataReceived(GtkWidget *treeview,
                                         GdkDragContext *, gint x,
                                         gint y, GtkSelectionData *data,
                                         guint , guint ,
                                         MainWindow *self) {
  GtkTreeModel *model;
  GtkTreePath *path;
  GtkTreeIter iter;
  gint bx, by;
  GroupInfo *grpinf;
  PalInfo *pal;
  SessionAbstract *session;
  GSList *list;

  /* 事件是否可用 */
  gtk_tree_view_convert_widget_to_bin_window_coords(GTK_TREE_VIEW(treeview), x,
                                                    y, &bx, &by);
  if (!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), bx, by, &path,
                                     NULL, NULL, NULL))
    return;

  /* 获取好友群组信息数据 */
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
  gtk_tree_model_get_iter(model, &iter, path);
  gtk_tree_path_free(path);
  gtk_tree_model_get(model, &iter, 6, &pal, -1);
  if (!(grpinf = g_cthrd->GetPalRegularItem(pal))) return;

  /* 如果好友群组对话框尚未创建，则先创建对话框 */
  if (!(grpinf->dialog))
    DialogPeer::PeerDialogEntry(self->app, grpinf);
  else
    gtk_window_present(GTK_WINDOW(grpinf->dialog));
  /* 获取会话对象，并将数据添加到会话对象 */
  session = (SessionAbstract *)g_object_get_data(G_OBJECT(grpinf->dialog),
                                                 "session-class");
  list = selection_data_get_path(data);  //获取所有文件
  session->AttachEnclosure(list);
  g_slist_foreach(list, GFunc(g_free), NULL);
  g_slist_free(list);
  //        session->ShowEnclosure();
}

/**
 * 主窗口位置&大小改变的响应处理函数.
 * @param window 主窗口
 * @param event the GdkEventConfigure which triggered this signal
 * @param dtset data set
 * @return Gtk+库所需
 */
gboolean MainWindow::MWinConfigureEvent(GtkWidget *,
                                        GdkEventConfigure *event,
                                        MainWindow *self) {
  self->windowConfig.SetWidth(event->width)
      .SetHeight(event->height)
      .SaveToConfig(self->config);
  return FALSE;
}

/**
 * 分割面板的分割位置改变的响应处理函数.
 * @param paned paned
 * @param pspec he GParamSpec of the property which changed
 * @param dtset data set
 */
void MainWindow::PanedDivideChanged(GtkWidget *paned, GParamSpec *,
                                    MainWindow *self) {
  self->config->SetInt("mwin_main_paned_divide",
                      gtk_paned_get_position(GTK_PANED(paned)));
  self->config->Save();
}

gboolean MainWindow::onDeleteEvent(MainWindow *self) {
  return self->statusIcon->AlterInterfaceMode();
}

void MainWindow::InitThemeSublayerData() {
  GtkIconTheme *theme;
  GtkIconFactory *factory;
  GtkIconSet *set;
  GdkPixbuf *pixbuf;

  theme = gtk_icon_theme_get_default();

  factory = gtk_icon_factory_new();
  gtk_icon_factory_add_default(factory);
  if ((pixbuf = gtk_icon_theme_load_icon(theme, "iptux", 64,
                                         GtkIconLookupFlags(0), NULL))) {
    set = gtk_icon_set_new_from_pixbuf(pixbuf);
    gtk_icon_factory_add(factory, "iptux-logo-show", set);
    g_object_unref(pixbuf);
  }
  if ((pixbuf = gtk_icon_theme_load_icon(theme, "iptux-i", 64,
                                         GtkIconLookupFlags(0), NULL))) {
    set = gtk_icon_set_new_from_pixbuf(pixbuf);
    gtk_icon_factory_add(factory, "iptux-logo-hide", set);
    g_object_unref(pixbuf);
  }
  g_object_unref(factory);
}

class EventData {
public:
  EventData(MainWindow* window, shared_ptr<const Event> event) {
    this->window = window;
    this->event = event;
  }

  MainWindow* window;
  shared_ptr<const Event> event;
};

gboolean MainWindow::processEventCallback(gpointer data) {
  EventData* callback = (EventData*)data;
  callback->window->processEventInMainThread(callback->event);
  delete callback;
  return G_SOURCE_REMOVE;
}

gboolean MainWindow::onTransWindowDelete(MainWindow& self) {
  self.transWindow = nullptr;
  return FALSE;
}


// this function is run in corethread, so always use g_idle_add to update the ui
void MainWindow::processEvent(shared_ptr<const Event> event) {
  // deleted in `processEventCallback`
  EventData* callback = new EventData(this, event);
  gdk_threads_add_idle(processEventCallback, callback);
}

void MainWindow::processEventInMainThread(shared_ptr<const Event> _event) {
  EventType type = _event->getType();
  if(type == EventType ::NEW_PAL_ONLINE) {
    auto event = (const NewPalOnlineEvent*)(_event.get());
    auto ipv4 = event->getPalInfo()->ipv4;
    if(PaltreeContainItem(ipv4)) {
      UpdateItemToPaltree(ipv4);
    } else {
      AttachItemToPaltree(ipv4);
    }
    return;
  }

  if(type == EventType::PAL_OFFLINE) {
    auto event = (const PalOfflineEvent*)(_event.get());
    auto ipv4 = event->GetPalKey().GetIpv4();
    if(PaltreeContainItem(ipv4)) {
      DelItemFromPaltree(ipv4);
    }
  }

  if(type == EventType::ICON_UPDATE) {
    auto event = (const IconUpdateEvent*)(_event.get());
    auto ipv4 = event->GetPalKey().GetIpv4();
    UpdateItemToPaltree(ipv4);
  }

  if(type == EventType::NEW_MESSAGE) {
    auto event = (const NewMessageEvent*)(_event.get());
    auto para = event->getMsgPara();
    GroupInfo *grpinf = nullptr;
    SessionAbstract *session;

    /* 获取群组信息 */
    switch (para.btype) {
      case GROUP_BELONG_TYPE_REGULAR:
        grpinf = coreThread.GetPalRegularItem(para.pal.get());
        coreThread.PushItemToMsgline(grpinf);
        if(coreThread.getProgramData()->IsAutoOpenCharDialog()) {
          if (!(grpinf->dialog)) {
            DialogPeer::PeerDialogEntry(g_mwin->app, grpinf);
          } else {
            gtk_window_present(GTK_WINDOW(grpinf->dialog));
          }
        }
        if(coreThread.getUiProgramData()->IsMsgSoundEnabled()) {
          g_sndsys->Playing(coreThread.getProgramData()->msgtip);
        }
        break;
      case GROUP_BELONG_TYPE_SEGMENT:
        grpinf = coreThread.GetPalSegmentItem(para.pal.get());
        break;
      case GROUP_BELONG_TYPE_GROUP:
        grpinf = coreThread.GetPalGroupItem(para.pal.get());
        break;
      case GROUP_BELONG_TYPE_BROADCAST:
        grpinf = coreThread.GetPalBroadcastItem(para.pal.get());
        break;
      default:
        grpinf = nullptr;
        break;
    }

    /* 如果群组存在则插入消息 */
    /* 群组不存在是编程上的错误，请发送Bug报告 */
    if (grpinf) {
      coreThread.InsertMsgToGroupInfoItem(grpinf, &para);
      if (grpinf->dialog) {
        session = (SessionAbstract *)g_object_get_data(G_OBJECT(grpinf->dialog),
                                                       "session-class");
        session->OnNewMessageComing();
      }
    }

    return;
  }

  if(type == EventType::PASSWORD_REQUIRED) {
    auto event = (const PasswordRequiredEvent*)(_event.get());
    auto palKey = event->GetPalKey();
    auto pal = g_cthrd->GetPal(palKey);
    auto passwd = pop_obtain_shared_passwd(GTK_WINDOW(g_mwin->getWindow()), pal.get());
    if (passwd && *passwd != '\0') {
      g_cthrd->SendAskSharedWithPassword(palKey, passwd);
    }
    return;
  }

  if(type == EventType::PERMISSION_REQUIRED) {
    auto event = (const PermissionRequiredEvent*)(_event.get());
    auto pal = g_cthrd->GetPal(event->GetPalKey());
    auto permit = pop_request_shared_file(GTK_WINDOW(g_mwin->getWindow()), pal.get());
    if (permit) {
      g_cthrd->SendSharedFiles(pal);
    }
    return;
  }

  if(type == EventType::NEW_SHARE_FILE_FROM_FRIEND) {
    auto event = dynamic_cast<const NewShareFileFromFriendEvent*>(_event.get());
    CHECK_NOTNULL(event);
    auto file = new FileInfo(event->GetFileInfo());
    g_cthrd->PushItemToEnclosureList(file);
    return;
  }

  if(type == EventType::SEND_FILE_STARTED || type == EventType::RECV_FILE_STARTED) {
    auto event = CHECK_NOTNULL(dynamic_cast<const AbstractTaskIdEvent*>(_event.get()));
    auto taskId = event->GetTaskId();
    auto para = g_cthrd->GetTransTaskStat(taskId);
    if(!para.get()) {
      LOG_WARN("got task id %d, but no info in CoreThread", taskId);
      return;
    }
    g_mwin->UpdateItemToTransTree(*para);
    auto g_progdt = g_cthrd->getUiProgramData();
    if (g_progdt->IsAutoOpenFileTrans()) {
       g_mwin->OpenTransWindow();
    }
    return;
  }

  if(type == EventType::SEND_FILE_FINISHED || type == EventType::RECV_FILE_FINISHED) {
    auto event = CHECK_NOTNULL(dynamic_cast<const AbstractTaskIdEvent*>(_event.get()));
    auto taskId = event->GetTaskId();
    auto para = g_cthrd->GetTransTaskStat(taskId);
    g_mwin->UpdateItemToTransTree(*para);
    auto g_progdt = g_cthrd->getUiProgramData();
    if (para->isFinished() && FLAG_ISSET(g_progdt->sndfgs, 2)) {
      g_sndsys->Playing(g_progdt->transtip);
    }
    return;
  }

  LOG_WARN("unknown event type: %d", int(type));
}

void MainWindow::setCurrentGroupInfo(GroupInfo* groupInfo) {
  this->currentGroupInfo = CHECK_NOTNULL(groupInfo);
  switch(currentGroupInfo->type) {
  case GROUP_BELONG_TYPE_REGULAR:
    setActionSensitive("pal.send_message", true);
    setActionSensitive("pal.request_shared_resources", true);
    setActionSensitive("pal.change_info", true);
    setActionSensitive("pal.delete_pal", true);
    break;
  case GROUP_BELONG_TYPE_SEGMENT:
  case GROUP_BELONG_TYPE_GROUP:
  case GROUP_BELONG_TYPE_BROADCAST:
    setActionSensitive("pal.send_message", true);
    setActionSensitive("pal.request_shared_resources", false);
    setActionSensitive("pal.change_info", false);
    setActionSensitive("pal.delete_pal", false);
    break;
  default:
    setActionSensitive("pal.send_message", false);
    setActionSensitive("pal.request_shared_resources", false);
    setActionSensitive("pal.change_info", false);
    setActionSensitive("pal.delete_pal", false);
    break;
  }
}

void MainWindow::setActionSensitive(const std::string& actionName, bool sensitive) {
  g_simple_action_set_enabled(
    G_SIMPLE_ACTION(
      g_action_map_lookup_action(G_ACTION_MAP(window), actionName.c_str())
    ),
    sensitive
  );
}



}  // namespace iptux
