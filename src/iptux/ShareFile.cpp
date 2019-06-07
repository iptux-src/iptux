//
// C++ Implementation: ShareFile
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
#include "ShareFile.h"

#include <sys/stat.h>

#include "iptux/AnalogFS.h"
#include "iptux-core/deplib.h"
#include "iptux/dialog.h"
#include "iptux/global.h"
#include "iptux-core/support.h"
#include "iptux-core/utils.h"
#include "iptux/UiHelper.h"

using namespace std;

namespace iptux {

static void DragDataReceived(ShareFile *sfile, GdkDragContext *context,
                      gint x, gint y, GtkSelectionData *data,
                      guint info, guint time);
static GtkTreeModel* CreateFileModel();
static void FillFileModel(GtkTreeModel *model);
static GtkWidget* CreateAllArea(ShareFile* self);
static void ApplySharedData(ShareFile* self);
static GtkWidget * CreateFileTree(GtkTreeModel *model);
static void AddRegular(ShareFile *sfile);
static void DeleteFiles(ShareFile* self);
static void AddFolder(ShareFile *sfile);
static void ClearPassword(ShareFile* self);
static void SetPassword(ShareFile* self);
static gint FileTreeCompareFunc(GtkTreeModel *model, GtkTreeIter *a,
                         GtkTreeIter *b);

void share_file_run(ShareFile* dialog) {
  switch (gtk_dialog_run(GTK_DIALOG(dialog))) {
    case GTK_RESPONSE_OK:
      ApplySharedData(dialog);
      break;
    case GTK_RESPONSE_APPLY:
      ApplySharedData(dialog);
      share_file_run(dialog);
      break;
    default:
      break;
  }
  gtk_widget_hide(GTK_WIDGET(dialog));
}

/**
 * 初始化底层数据.
 */
void InitSublayer(ShareFile* self) {
  GtkTreeModel *model;

  model = CreateFileModel();
  g_object_set_data_full(G_OBJECT(self), "file-model", model,
                           GDestroyNotify(g_object_unref));
  FillFileModel(model);
}

/**
 * 创建主对话框窗口.
 * @param parent 父窗口指针
 * @return 对话框
 */
ShareFile* share_file_new(GtkWindow* parent) {
  ShareFile* dialog;

  dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(
      _("Shared Files Management"), GTK_WINDOW(parent),
      GTK_DIALOG_MODAL, _("OK"), GTK_RESPONSE_OK, _("Apply"),
      GTK_RESPONSE_APPLY, _("Cancel"), GTK_RESPONSE_CANCEL, NULL));

  InitSublayer(dialog);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
  gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(dialog), 5);
  gtk_widget_set_size_request(GTK_WIDGET(dialog), 500, 350);

  widget_enable_dnd_uri(GTK_WIDGET(dialog));
  g_signal_connect(dialog, "drag-data-received",
                           G_CALLBACK(DragDataReceived), NULL);

  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                     CreateAllArea(dialog), TRUE, TRUE, 0);
  return dialog;
}

/**
 * 创建对话框中所有的窗体.
 * @return 主窗体
 */
GtkWidget* CreateAllArea(ShareFile* self) {
  GtkWidget *box, *vbox;
  GtkWidget *sw, *button, *widget;
  GtkTreeModel *model;

  box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

  /* 加入文件树 */
  sw = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                      GTK_SHADOW_ETCHED_IN);
  gtk_box_pack_end(GTK_BOX(box), sw, TRUE, TRUE, 0);
  model = GTK_TREE_MODEL(g_object_get_data(G_OBJECT(self), "file-model"));
  widget = CreateFileTree(model);
  gtk_container_add(GTK_CONTAINER(sw), widget);
  g_object_set_data(G_OBJECT(self), "file-treeview-widget", widget);

  /* 加入N多按钮 */
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(box), vbox, FALSE, FALSE, 0);
  button = gtk_button_new_with_label(_("Add Files"));
  gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
  g_signal_connect_swapped(button, "clicked", G_CALLBACK(AddRegular), self);
  button = gtk_button_new_with_label(_("Add Folders"));
  gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
  g_signal_connect_swapped(button, "clicked", G_CALLBACK(AddFolder), self);
  button = gtk_button_new_with_label(_("Delete Resources"));
  gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
  g_signal_connect_swapped(button, "clicked", G_CALLBACK(DeleteFiles), self);
  button = gtk_button_new_with_label(_("Clear Password"));
  gtk_box_pack_end(GTK_BOX(vbox), button, FALSE, FALSE, 0);
  g_signal_connect_swapped(button, "clicked", G_CALLBACK(ClearPassword),
                           self);
  button = gtk_button_new_with_label(_("Set Password"));
  gtk_box_pack_end(GTK_BOX(vbox), button, FALSE, FALSE, 0);
  g_signal_connect_swapped(button, "clicked", G_CALLBACK(SetPassword), self);
  g_object_set_data(G_OBJECT(self), "password-button-widget", button);
  gtk_widget_show_all(box);
  return box;
}


enum {
  COLUMN_ICONNAME,
  COLUMN_FILENAME,
  COLUMN_FILESIZE,
  COLUMN_FILETYPE,
  COLUMN_TYPE,
  N_COLUMNS
};


/**
 * 文件树(file-tree)底层数据结构.
 * 5,0 logo,1 filepath,2 filesize,3 filetype,4 type
 * 文件图标;文件路径;文件大小;文件类型(串);文件类型(数值)
 * @return file-model
 */
GtkTreeModel* CreateFileModel() {
  GtkListStore *model;

  model = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                             G_TYPE_STRING, G_TYPE_UINT);
  gtk_tree_sortable_set_default_sort_func(
      GTK_TREE_SORTABLE(model), GtkTreeIterCompareFunc(FileTreeCompareFunc),
      NULL, NULL);
  gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model),
                                       GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
                                       GTK_SORT_ASCENDING);

  return GTK_TREE_MODEL(model);
}

/**
 * 为文件树(file-tree)填充底层数据.
 * @param model file-model
 */
void FillFileModel(GtkTreeModel *model) {
  AnalogFS afs;
  const char *iconname;
  GtkTreeIter iter;
  char *filesize;
  const char *filetype;
  FileInfo *file;

  /* 将现在的共享文件填入model */
  for(FileInfo& file: g_cthrd->getProgramData()->GetSharedFileInfos()) {
    /* 获取文件大小 */
    file.filesize = afs.ftwsize(file.filepath);
    filesize = numeric_to_size(file.filesize);
    /* 获取文件类型 */
    switch (GET_MODE(file.fileattr)) {
      case IPMSG_FILE_REGULAR:
        filetype = _("regular");
        iconname = "text-x-generic-symbolic";
        break;
      case IPMSG_FILE_DIR:
        filetype = _("directory");
        iconname = "folder-symbolic";
        break;
      default:
        filetype = _("unknown");
        iconname = NULL;
        break;
    }
    /* 填入数据 */
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                       0, iconname,
                       1, file.filepath,
                       2, filesize,
                       3, filetype,
                       4, file.fileattr,
                       -1);
    /* 烦，释放资源 */
    g_free(filesize);
  }
}

/**
 * 创建文件树(file-tree).
 * @param model file-model
 * @return 文件树
 */
GtkWidget * CreateFileTree(GtkTreeModel *model) {
  GtkWidget *view;
  GtkTreeViewColumn *column;
  GtkCellRenderer *cell;
  GtkTreeSelection *selection;

  view = gtk_tree_view_new_with_model(model);
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), TRUE);
  gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(view), TRUE);
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
  gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

  column = gtk_tree_view_column_new();
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_column_set_title(column, _("File"));
  cell = gtk_cell_renderer_pixbuf_new();
  gtk_tree_view_column_pack_start(column, cell, FALSE);
  gtk_tree_view_column_set_attributes(column, cell, "icon-name", 0, NULL);
  cell = gtk_cell_renderer_text_new();
  gtk_tree_view_column_pack_start(column, cell, FALSE);
  gtk_tree_view_column_set_attributes(column, cell, "text", 1, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Size"), cell, "text", 2,
                                                    NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Type"), cell, "text", 3,
                                                    NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  return view;
}

/**
 * 应用共享文件数据.
 */
void ApplySharedData(ShareFile* self) {
  GtkWidget *widget;
  GtkTreeModel *model;
  GtkTreeIter iter;
  FileInfo *file;
  uint32_t fileattr;
  gchar *filepath;
  const gchar *passwd;
  AnalogFS afs;
  struct stat st;

  /* 更新共享文件链表 */
  g_cthrd->Lock();
  g_cthrd->getProgramData()->ClearShareFileInfos();
  g_cthrd->PbnQuote() = 1;
  widget = GTK_WIDGET(g_object_get_data(G_OBJECT(self), "file-treeview-widget"));
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  if (gtk_tree_model_get_iter_first(model, &iter)) {
    do {
      gtk_tree_model_get(model, &iter, 1, &filepath, 4, &fileattr, -1);
      FileInfo file;
      file.fileid = g_cthrd->PbnQuote()++;
      file.fileattr = fileattr;
      file.filepath = filepath;
      if (afs.stat(filepath, &st) == 0) {
        file.filectime = st.st_ctime;
      }
      g_cthrd->getProgramData()->AddShareFileInfo(move(file));
    } while (gtk_tree_model_iter_next(model, &iter));
  }
  g_cthrd->Unlock();

  /* 更新密码 */
  widget = GTK_WIDGET(g_object_get_data(G_OBJECT(self), "password-button-widget"));
  passwd = (const gchar *)g_object_get_data(G_OBJECT(widget), "password");
  if(!passwd) {
    g_cthrd->SetAccessPublicLimit("");
  } else {
    g_cthrd->SetAccessPublicLimit(passwd);
  }
  g_cthrd->getUiProgramData()->WriteProgData();
}

/**
 * 增加新的共享文件.
 * @param list 文件链表
 */
void AttachSharedFiles(ShareFile* self, GSList *list) {
  AnalogFS afs;
  GtkWidget *widget;
  GtkTreeModel *model;
  GtkTreeIter iter;
  const char *iconname;
  struct stat st;
  int64_t pathsize;
  GSList *tlist;
  char *filesize;
  const char *filetype;
  uint32_t fileattr;

  /* 插入文件树 */
  widget = GTK_WIDGET(g_object_get_data(G_OBJECT(self), "file-treeview-widget"));
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  tlist = list;
  while (tlist) {
    if (stat((const char *)tlist->data, &st) == -1 ||
        !(S_ISREG(st.st_mode) || S_ISDIR(st.st_mode))) {
      tlist = g_slist_next(tlist);
      continue;
    }
    /* 获取文件大小 */
    pathsize = afs.ftwsize((const char *)tlist->data);
    filesize = numeric_to_size(pathsize);
    /* 获取文件类型 */
    if (S_ISREG(st.st_mode)) {
      filetype = _("regular");
      fileattr = IPMSG_FILE_REGULAR;
      iconname = "text-x-generic-symbolic";
    } else if (S_ISDIR(st.st_mode)) {
      filetype = _("directory");
      fileattr = IPMSG_FILE_DIR;
      iconname = "folder-symbolic";
    } else {
      filetype = _("unknown");
      fileattr = 0;
      iconname = NULL;
    }
    /* 添加数据 */
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                       0, iconname,
                       1, tlist->data,
                       2, filesize,
                       3, filetype,
                       4, fileattr,
                       -1);
    /* 释放资源 */
    g_free(filesize);
    /* 转到下一个节点 */
    tlist = g_slist_next(tlist);
  }
}

/**
 * 选择新的共享文件.
 * @param fileattr 文件类型
 * @return 文件链表
 */
GSList * PickSharedFile(ShareFile* self, uint32_t fileattr) {
  GtkWidget *dialog;
  GtkFileChooserAction action;
  const char *title;
  GSList *list;

  if (GET_MODE(fileattr) == IPMSG_FILE_REGULAR) {
    action = GTK_FILE_CHOOSER_ACTION_OPEN;
    title = _("Choose the files to share");
  } else {
    action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
    title = _("Choose the folders to share");
  }

  dialog = gtk_file_chooser_dialog_new(
      title, GTK_WINDOW(self), action,
      _("_Open"), GTK_RESPONSE_ACCEPT,
      _("_Cancel"), GTK_RESPONSE_CANCEL,
      NULL);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
  gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), FALSE);
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
  gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
                                      g_get_home_dir());

  switch (gtk_dialog_run(GTK_DIALOG(dialog))) {
    case GTK_RESPONSE_ACCEPT:
      list = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
      break;
    case GTK_RESPONSE_CANCEL:
    default:
      list = NULL;
      break;
  }
  gtk_widget_destroy(dialog);

  return list;
}

/**
 * 增添常规文件.
 * @param sfile 共享文件类
 */
void AddRegular(ShareFile *sfile) {
  GSList *list;

  list = PickSharedFile(sfile, IPMSG_FILE_REGULAR);
  AttachSharedFiles(sfile, list);
  g_slist_foreach(list, GFunc(g_free), NULL);
  g_slist_free(list);
}

/**
 * 增添目录文件.
 * @param sfile 共享文件类
 */
void AddFolder(ShareFile *sfile) {
  GSList *list;

  list = PickSharedFile(sfile, IPMSG_FILE_DIR);
  AttachSharedFiles(sfile, list);
  g_slist_foreach(list, GFunc(g_free), NULL);
  g_slist_free(list);
}

/**
 * 删除被选中的共享文件.
 * @param widset widget set
 */
void DeleteFiles(ShareFile* self) {
  GtkWidget *widget;
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter;

  widget = GTK_WIDGET(g_object_get_data(G_OBJECT(self), "file-treeview-widget"));
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  if (!gtk_tree_model_get_iter_first(model, &iter)) return;
  do {
    if (gtk_tree_selection_iter_is_selected(selection, &iter)) {
      gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
    }
  } while (gtk_tree_model_iter_next(model, &iter));
}

void SetPassword(ShareFile* self) {
  GtkWidget *widget;
  char *passwd, *epasswd;

  if (!(passwd = pop_password_settings(GTK_WIDGET(self)))) {
    return;
  }
  widget = GTK_WIDGET(g_object_get_data(G_OBJECT(self), "password-button-widget"));
  epasswd = g_base64_encode((guchar *)passwd, strlen(passwd));
  g_object_set_data_full(G_OBJECT(widget), "password", epasswd,
                         GDestroyNotify(g_free));
  g_free(passwd);
}

void ClearPassword(ShareFile* self) {
  GtkWidget *widget;

  widget = GTK_WIDGET(g_object_get_data(G_OBJECT(self), "password-button-widget"));
  g_object_set_data(G_OBJECT(widget), "password", NULL);
}

/**
 * 接收拖拽文件信息.
 * @param sfile 共享文件类
 * @param context the drag context
 * @param x where the drop happened
 * @param y where the drop happened
 * @param data the received data
 * @param info the info that has been registered with the target in the
 * GtkTargetList
 * @param time the timestamp at which the data was received
 */
void DragDataReceived(ShareFile *sfile, GdkDragContext *context,
                                 gint x, gint y, GtkSelectionData *data,
                                 guint info, guint time) {
  GSList *list;
  if (!ValidateDragData(data, context, time)) {
    return;
  }

  list = selection_data_get_path(data);
  AttachSharedFiles(sfile, list);
  g_slist_foreach(list, GFunc(g_free), NULL);
  g_slist_free(list);

  gtk_drag_finish(context, TRUE, FALSE, time);
}

/**
 * 文件树(file-tree)排序比较函数.
 * @param model network-model
 * @param a A GtkTreeIter in model
 * @param b Another GtkTreeIter in model
 * @return 比较值
 */
gint FileTreeCompareFunc(GtkTreeModel *model, GtkTreeIter *a,
                                    GtkTreeIter *b) {
  gchar *afilepath, *bfilepath;
  uint32_t afileattr, bfileattr;
  gint result;

  gtk_tree_model_get(model, a, 1, &afilepath, 4, &afileattr, -1);
  gtk_tree_model_get(model, b, 1, &bfilepath, 4, &bfileattr, -1);
  if (GET_MODE(afileattr) == GET_MODE(bfileattr))
    result = strcmp(afilepath, bfilepath);
  else if (GET_MODE(afileattr) == IPMSG_FILE_REGULAR)
    result = 1;
  else
    result = -1;
  g_free(afilepath);
  g_free(bfilepath);

  return result;
}

}  // namespace iptux
