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
  GtkWidget* transTreeviewWidget;

 public:
  static void destroy(TransWindowPrivate* self) {
    delete self;
  }
};

static gboolean TWinConfigureEvent(GtkWindow *window);
static GtkWidget * CreateTransArea(GtkWindow* window);
static GtkWidget* CreateTransTree(TransWindow *window);
static GtkWidget *CreateTransPopupMenu(GtkTreeModel *model);
static void OpenThisFile(GtkTreeModel *model);
static void ClearTransTask(GtkTreeModel *model);
static gboolean UpdateTransUI(GtkWindow *window);
static TransWindowPrivate& getPriv(TransWindow* window);
static shared_ptr<IptuxConfig> trans_window_get_config(GtkWindow *pWindow);

TransWindow *trans_window_new(GtkWindow *parent) {
  g_assert(g_object_get_data(G_OBJECT(parent), "iptux-config") != nullptr);
  g_assert(g_object_get_data(G_OBJECT(parent), "trans-model") != nullptr);
  g_assert(g_action_map_lookup_action(G_ACTION_MAP(parent), "trans_model_changed") != nullptr);

  GtkWindow *window;

  window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
  TransWindowPrivate* priv = new TransWindowPrivate;
  g_object_set_data_full(G_OBJECT(window), IPTUX_PRIVATE, priv, GDestroyNotify(TransWindowPrivate::destroy));
  gtk_window_set_transient_for(window, parent);
  gtk_window_set_destroy_with_parent(window, true);

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
  g_signal_connect_swapped(
      g_action_map_lookup_action(G_ACTION_MAP(parent), "trans_model_changed"),
      "activate",
      G_CALLBACK(UpdateTransUI),
      window
  );
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
  GtkWindow* parent = gtk_window_get_transient_for(window);
  return *(static_cast<shared_ptr<IptuxConfig> *>(g_object_get_data(G_OBJECT(parent), "iptux-config")));
}

GtkTreeModel* trans_window_get_trans_model(GtkWindow* window) {
  GtkWindow* parent = gtk_window_get_transient_for(window);
  return GTK_TREE_MODEL(g_object_get_data(G_OBJECT(parent), "trans-model"));
}


/**
 * 清理文件传输任务.
 * @param widset widget set
 */
static void ClearTransWindow(GtkWidget *window) {
  GtkWidget *treeview;
  GtkTreeModel *model;

  /* 考察是否需要清理UI */
  treeview = getPriv(GTK_WINDOW(window)).transTreeviewWidget;
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
  ClearTransTask(model);

  /* 重新调整UI */
  gtk_tree_view_columns_autosize(GTK_TREE_VIEW(treeview));
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

  hbb = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_button_box_set_layout(GTK_BUTTON_BOX(hbb), GTK_BUTTONBOX_END);
  gtk_box_pack_start(GTK_BOX(box), hbb, FALSE, FALSE, 0);
  button = gtk_button_new_with_label(_("Clear"));
  gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_icon_name("edit-clear-symbolic", GTK_ICON_SIZE_BUTTON));
  gtk_box_pack_start(GTK_BOX(hbb), button, FALSE, FALSE, 0);
  g_signal_connect_swapped(button, "clicked", G_CALLBACK(ClearTransWindow), window);
  gtk_widget_show_all(box);
  return box;
}

/**
 * 文件传输树(trans-tree)弹出操作菜单.
 * @param treeview tree-view
 * @param event event
 * @return Gtk+库所需
 */
static gboolean TransPopupMenu(GtkWidget *treeview,
                                    GdkEventButton *event) {
  GtkWidget *menu;
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
  }
  /* 弹出菜单 */
  menu = CreateTransPopupMenu(model);
  if(menu == nullptr) {
    return true;
  }
  gtk_widget_show_all(menu);
  gtk_menu_popup_at_pointer(GTK_MENU(menu), NULL);
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
  g_signal_connect(view, "button-press-event", G_CALLBACK(TransPopupMenu),
                   NULL);

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
 * 清理文件传输任务.
 * @param model trans-model
 */
void ClearTransTask(GtkTreeModel *model) {
  GtkTreeIter iter;
  int taskId;

  if (!gtk_tree_model_get_iter_first(model, &iter)) return;
  do {
    gtk_tree_model_get(model, &iter, TransModelColumn ::TASK_ID, &taskId, -1);
    // TODO: clear finished task
    // if (!data) {
    //   if (gtk_list_store_remove(GTK_LIST_STORE(model), &iter)) goto mark;
    //   break;
    // }
  } while (gtk_tree_model_iter_next(model, &iter));
}


/**
 * 为文件传输树(trans-tree)创建弹出菜单.
 * @param model trans-model
 * @return 菜单
 */
GtkWidget *CreateTransPopupMenu(GtkTreeModel *model) {
  GtkWidget *menu, *menuitem;

  GtkTreePath *path;
  GtkTreeIter iter;
  bool finished;

  if (!(path = (GtkTreePath *)(g_object_get_data(G_OBJECT(model),
                                                 "selected-path"))))
    return NULL;
  gtk_tree_model_get_iter(model, &iter, path);
  gtk_tree_model_get(model, &iter, TransModelColumn ::FINISHED, &finished, -1);

  menu = gtk_menu_new();

  menuitem = gtk_menu_item_new_with_label(_("Open This File"));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
  g_signal_connect_swapped(menuitem, "activate", G_CALLBACK(OpenThisFile),
                           model);
  gtk_widget_set_sensitive(GTK_WIDGET(menuitem), finished);

  menuitem = gtk_menu_item_new_with_label(_("Open Containing Folder"));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
  g_signal_connect_swapped(menuitem, "activate",
                           G_CALLBACK(OpenContainingFolder), model);
  gtk_widget_set_sensitive(GTK_WIDGET(menuitem), finished);

  menuitem = gtk_menu_item_new_with_label(_("Terminate Task"));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
  g_signal_connect_swapped(menuitem, "activate", G_CALLBACK(TerminateTransTask),
                           model);
  gtk_widget_set_sensitive(GTK_WIDGET(menuitem), !finished);

  menuitem = gtk_menu_item_new_with_label(_("Terminate All"));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
  g_signal_connect_swapped(menuitem, "activate",
                           G_CALLBACK(TerminateAllTransTask), model);

  menuitem = gtk_menu_item_new_with_label(_("Clear Tasklist"));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
  g_signal_connect_swapped(menuitem, "activate", G_CALLBACK(ClearTransTask),
                           model);

  return menu;
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
