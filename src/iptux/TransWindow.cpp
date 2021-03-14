#include "config.h"
#include "TransWindow.h"

#include <memory>
#include <glib/gi18n.h>

#include "iptux-core/IptuxConfig.h"
#include "iptux-utils/utils.h"

#include "iptux-utils/output.h"
#include "iptux/UiModels.h"
#include "iptux/UiHelper.h"
#include "iptux/global.h"

#define IPTUX_PRIVATE "iptux-private"

using namespace std;

namespace iptux {

class TransWindowPrivate {
 public:
  Application* app;
  GtkWidget* transTreeviewWidget;
  GtkMenu* popupMenu;
  vector<gulong> signals;
  GtkTreeModel* model;
  GtkApplicationWindow* window;

 public:
  static void destroy(TransWindowPrivate* self) {
    for(gulong i: self->signals) {
      g_signal_handler_disconnect(
        g_action_map_lookup_action(G_ACTION_MAP(self->app->getApp()),"trans_model.changed"),
        i);
    }
    delete self;
  }

  void setCurrentTaskFinished(bool finished) {
    currentTaskFinished = finished;
    if(finished) {
      g_action_map_enable_actions(G_ACTION_MAP(window), "trans.open_file", nullptr);
      g_action_map_disable_actions(G_ACTION_MAP(window), "trans.terminate_task", nullptr);
    } else {
      g_action_map_disable_actions(G_ACTION_MAP(window), "trans.open_file", nullptr);
      g_action_map_enable_actions(G_ACTION_MAP(window), "trans.terminate_task", nullptr);
    }
  }

  private:
    bool currentTaskFinished = false;

};

static gboolean TWinConfigureEvent(GtkWindow *window);
static GtkWidget * CreateTransArea(GtkWindow* window);
static GtkWidget* CreateTransTree(TransWindow *window);
static void OpenThisFile(GtkTreeModel *model);
static gboolean UpdateTransUI(GtkWindow *window);
static TransWindowPrivate& getPriv(TransWindow* window);
static shared_ptr<IptuxConfig> trans_window_get_config(GtkWindow *pWindow);
static void OpenContainingFolder(GtkTreeModel *model);
static void TerminateTransTask(GtkTreeModel *model);
static void TerminateAllTransTask(GtkTreeModel *model);
static void onOpenFile (void *, void *, TransWindowPrivate* self) {
  OpenThisFile(self->model);
}
static void onOpenFolder (void *, void *, TransWindowPrivate* self) {
  OpenContainingFolder(self->model);
}
static void onTerminateTask (void *, void *, TransWindowPrivate* self) {
  TerminateTransTask(self->model);
}
static void onTerminateAllTasks (void *, void *, TransWindowPrivate* self) {
  TerminateAllTransTask(self->model);
}

TransWindow *trans_window_new(Application* app, GtkWindow *parent) {
  g_assert(app != nullptr);
  GtkWindow *window;

  window = GTK_WINDOW(gtk_application_window_new(app->getApp()));
  TransWindowPrivate* priv = new TransWindowPrivate;
  priv->app = app;
  priv->window = GTK_APPLICATION_WINDOW(window);
  g_object_set_data_full(G_OBJECT(window), IPTUX_PRIVATE, priv, GDestroyNotify(TransWindowPrivate::destroy));
  if(parent) {
    gtk_window_set_transient_for(window, parent);
    gtk_window_set_destroy_with_parent(window, true);
  }

  auto config = trans_window_get_config(window);
  gtk_window_set_title(GTK_WINDOW(window), _("Files Transmission Management"));
  gtk_window_set_default_size(GTK_WINDOW(window),
                              config->GetInt("trans_window_width", 500),
                              config->GetInt("trans_window_height", 350));
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(window), 5);
  gtk_container_add(GTK_CONTAINER(window), CreateTransArea(window));

  g_signal_connect(window, "delete-event", G_CALLBACK(gtk_widget_hide), NULL);
  g_signal_connect(window, "configure-event", G_CALLBACK(TWinConfigureEvent), NULL);
  auto signalHandler = g_signal_connect_swapped(
      g_action_map_lookup_action(G_ACTION_MAP(app->getApp()), "trans_model.changed"),
      "activate",
      G_CALLBACK(UpdateTransUI),
      window
  );
  priv->signals.push_back(signalHandler);

  GActionEntry win_entries[] = {
    { "trans.open_file", G_ACTION_CALLBACK(onOpenFile)},
    { "trans.open_folder", G_ACTION_CALLBACK(onOpenFolder)},
    { "trans.terminate_task", G_ACTION_CALLBACK(onTerminateTask)},
    { "trans.terminate_all", G_ACTION_CALLBACK(onTerminateAllTasks)},
  };

  g_action_map_add_action_entries (G_ACTION_MAP (window),
                                   win_entries, G_N_ELEMENTS (win_entries),
                                   priv);

  priv->popupMenu = GTK_MENU(gtk_menu_new_from_model(
    G_MENU_MODEL(gtk_builder_get_object(app->getMenuBuilder(), "trans-popup"))
  ));
  gtk_menu_attach_to_widget(priv->popupMenu, GTK_WIDGET(window), nullptr);
  return window;
}

/**
 * 文件传输窗口位置&大小改变的响应处理函数.
 * @param window 文件传输窗口
 * @param event the GdkEventConfigure which triggered this signal
 * @param dtset data set
 * @return Gtk+库所需
 */
gboolean TWinConfigureEvent(GtkWindow *window) {
  int width, height;
  gtk_window_get_size(window, &width, &height);

  auto config = trans_window_get_config(window);
  config->SetInt("trans_window_width", width);
  config->SetInt("trans_window_height", height);
  config->Save();
  return FALSE;
}

shared_ptr<IptuxConfig> trans_window_get_config(GtkWindow *window) {
  return getPriv(window).app->getConfig();
}

TransModel* trans_window_get_trans_model(GtkWindow* window) {
  return getPriv(window).app->getTransModel();
}

/**
 * 创建文件传输窗口其他区域.
 * @return 主窗体
 */
GtkWidget * CreateTransArea(GtkWindow* window) {
  GtkWidget *box, *hbb;
  GtkWidget *sw, *button, *widget;

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  sw = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                      GTK_SHADOW_ETCHED_IN);
  gtk_box_pack_start(GTK_BOX(box), sw, TRUE, TRUE, 0);
  widget = CreateTransTree(window);
  gtk_container_add(GTK_CONTAINER(sw), widget);
  getPriv(window).transTreeviewWidget = widget;
  getPriv(window).model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));

  hbb = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_button_box_set_layout(GTK_BUTTON_BOX(hbb), GTK_BUTTONBOX_END);
  gtk_box_pack_start(GTK_BOX(box), hbb, FALSE, FALSE, 0);
  button = gtk_button_new_with_label(_("Clear"));
  gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_icon_name("edit-clear-symbolic", GTK_ICON_SIZE_BUTTON));
  gtk_box_pack_start(GTK_BOX(hbb), button, FALSE, FALSE, 0);
  gtk_actionable_set_action_name(GTK_ACTIONABLE(button), "app.trans_model.clear");
  gtk_widget_show_all(box);
  return box;
}

/**
 * 文件传输树(trans-tree)弹出操作菜单.
 * @param treeview tree-view
 * @param event event
 * @return Gtk+库所需
 */
static gboolean TransPopupMenu(
  GtkWidget *treeview,
  GdkEventButton *event,
  TransWindowPrivate* priv)
{
  GtkTreeModel *model;
  GtkTreePath *path;

  /* 检查事件是否可用 */
  if (event->button != GDK_BUTTON_SECONDARY) {
    return FALSE;
  }

  /* 确定当前被选中的路径 */
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
  if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), (event->x),
                                    (event->y), &path, NULL, NULL, NULL)) {
    g_object_set_data_full(G_OBJECT(model), "selected-path", path,
                           GDestroyNotify(gtk_tree_path_free));
  } else {
    g_object_set_data(G_OBJECT(model), "selected-path", NULL);
    return TRUE;
  }

  GtkTreeIter iter;
  gtk_tree_model_get_iter(model, &iter, path);
  bool finished;
  gtk_tree_model_get(model, &iter, TransModelColumn ::FINISHED, &finished, -1);
  priv->setCurrentTaskFinished(finished);

  // /* 弹出菜单 */
  // menu = CreateTransPopupMenu(model);
  // if(menu == nullptr) {
  //   return true;
  // }
  // gtk_menu_attach_to_widget(GTK_MENU(menu), treeview, nullptr);
  // gtk_widget_show_all(menu);
  gtk_menu_popup_at_pointer(GTK_MENU(priv->popupMenu), NULL);
  return TRUE;
}


/**
 * 创建文件传输树(trans-tree).
 * @param model trans-model
 * @return 传输树
 */
GtkWidget* CreateTransTree(TransWindow *window) {
  GtkWidget *view;
  GtkTreeViewColumn *column;
  GtkCellRenderer *cell;
  GtkTreeSelection *selection;

  auto model = trans_window_get_trans_model(window);
  view = gtk_tree_view_new_with_model(model);
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), TRUE);
  gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(view), TRUE);
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
  gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
  g_signal_connect(view, "button-press-event", G_CALLBACK(TransPopupMenu), &getPriv(window));

  cell = gtk_cell_renderer_pixbuf_new();
  column = gtk_tree_view_column_new_with_attributes(_("State"), cell,
                                                    "icon-name", TransModelColumn::STATUS,
                                                    NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Task"), cell,
                                                    "text", TransModelColumn::TASK,
                                                    NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Peer"), cell,
                                                    "text", TransModelColumn::PEER,
                                                    NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("IPv4"), cell,
                                                    "text", TransModelColumn::IP,
                                                    NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Filename"), cell,
                                                    "text", TransModelColumn::FILENAME,
                                                    NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Size"), cell,
                                                    "text", TransModelColumn::FILE_LENGTH_TEXT,
                                                    NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Completed"), cell,
                                                    "text", TransModelColumn::FINISHED_LENGTH_TEXT,
                                                    NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_progress_new();
  column = gtk_tree_view_column_new_with_attributes(_("Progress"), cell,
                                                    "value", TransModelColumn::PROGRESS,
                                                    "text", TransModelColumn::PROGRESS_TEXT,
                                                    NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Cost"), cell,
                                                    "text", TransModelColumn::COST,
                                                    NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Remaining"), cell,
                                                    "text", TransModelColumn::REMAIN,
                                                    NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Rate"), cell,
                                                    "text", TransModelColumn::RATE,
                                                    NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  return view;
}

/**
 * 打开接收文件所在文件夹.
 * @param model trans-model
 */
static void OpenContainingFolder(GtkTreeModel *model) {
  GtkTreePath *path;
  GtkTreeIter iter;
  gchar *filename, *filepath, *name;
  if (!(path = (GtkTreePath *)(g_object_get_data(G_OBJECT(model),
                                                 "selected-path"))))
    return;
  gtk_tree_model_get_iter(model, &iter, path);
  gtk_tree_model_get(model, &iter, TransModelColumn::FILE_PATH, &filename, -1);
  if (filename) {
    name = ipmsg_get_filename_me(filename, &filepath);
    if (!g_file_test(filepath, G_FILE_TEST_EXISTS)) {
      GtkWidget *dialog = gtk_message_dialog_new(
          NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s",
          _("The path you want to open not exist!"));
      gtk_window_set_title(GTK_WINDOW(dialog), "Iptux Error");
      gtk_dialog_run(GTK_DIALOG(dialog));
      gtk_widget_destroy(dialog);
      return;
    }
    iptux_open_url(filepath);
    g_free(name);
    g_free(filepath);
  }
}

/**
 * 终止单个传输任务.
 * @param model trans-model
 */
static void TerminateTransTask(GtkTreeModel *model) {
  GtkTreePath *path;
  GtkTreeIter iter;
  gboolean finished;
  int taskId;

  if (!(path = (GtkTreePath *)(g_object_get_data(G_OBJECT(model),
                                                 "selected-path"))))
    return;
  gtk_tree_model_get_iter(model, &iter, path);
  gtk_tree_model_get(model, &iter,
                     TransModelColumn::TASK_ID, &taskId,
                     TransModelColumn::FINISHED, &finished,
                     -1);
  if(finished) {
    return;
  }
  g_cthrd->TerminateTransTask(taskId);
}


/**
 * 终止所有传输任务.
 * @param model trans-model
 */
static void TerminateAllTransTask(GtkTreeModel *model) {
  GtkTreeIter iter;
  int taskId;

  if (!gtk_tree_model_get_iter_first(model, &iter)) return;
  do {
    gtk_tree_model_get(model, &iter, TransModelColumn ::TASK_ID, &taskId, -1);
    g_cthrd->TerminateTransTask(taskId);
  } while (gtk_tree_model_iter_next(model, &iter));
}


/**
 * 打开接收的文件.
 * @param model trans-model
 */
void OpenThisFile(GtkTreeModel *model) {
  GtkTreePath *path;
  GtkTreeIter iter;
  gchar *filename;

  if (!(path = (GtkTreePath *)(g_object_get_data(G_OBJECT(model),
                                                 "selected-path"))))
    return;
  gtk_tree_model_get_iter(model, &iter, path);
  gtk_tree_model_get(model, &iter, TransModelColumn ::FILE_PATH, &filename, -1);
  if (filename) {
    if (!g_file_test(filename, G_FILE_TEST_EXISTS)) {
      GtkWidget *dialog = gtk_message_dialog_new(
          NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s",
          _("The file you want to open not exist!"));
      gtk_window_set_title(GTK_WINDOW(dialog), _("iptux Error"));
      gtk_dialog_run(GTK_DIALOG(dialog));
      gtk_widget_destroy(dialog);
      return;
    }
    iptux_open_url(filename);
  }
}

/**
 * 更新文件传输窗口UI.
 * @param treeview tree-view
 * @return GLib库所需
 */
gboolean UpdateTransUI(GtkWindow *window) {
  GtkTreeModel *model;
  GtkTreeIter iter;

  GtkWidget* treeview = getPriv(window).transTreeviewWidget;

  /* 考察是否需要更新UI */
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
  if (!gtk_tree_model_get_iter_first(model, &iter)) return TRUE;

  /* 更新UI */
  int taskId;
  do {
    gtk_tree_model_get(model, &iter, TransModelColumn ::TASK_ID, &taskId, -1);
    auto transFileModel = g_cthrd->GetTransTaskStat(taskId);
    transModelFillFromTransFileModel(model, &iter, *transFileModel);
  } while (gtk_tree_model_iter_next(model, &iter));

  /* 重新调整UI */
  gtk_tree_view_columns_autosize(GTK_TREE_VIEW(treeview));

  return TRUE;
}

TransWindowPrivate &getPriv(TransWindow *window) {
  return *((TransWindowPrivate*)g_object_get_data(G_OBJECT(window), IPTUX_PRIVATE));
}

}
