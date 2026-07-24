//
// C++ Implementation: RevisePal
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
#include "RevisePal.h"

#include <cinttypes>
#include <dirent.h>
#include <glib/gi18n.h>

#include "iptux-core/Const.h"
#include "iptux-utils/utils.h"
#include "iptux/UiCoreThread.h"
#include "iptux/callback.h"

namespace iptux {

namespace {

void AttachLabel(GtkGrid* grid, GtkWidget* label, gint row) {
  gtk_label_set_xalign(GTK_LABEL(label), 1.0);
  gtk_widget_set_halign(label, GTK_ALIGN_END);
  gtk_grid_attach(grid, label, 0, row, 1, 1);
}

void AppendProtocolOption(GtkComboBoxText* combo, const char* option) {
  gtk_combo_box_text_append_text(combo, option);
}

gint ProtocolToActive(PalProtocol protocol) {
  switch (protocol) {
    case PalProtocol::AUTO:
      return 0;
    case PalProtocol::IPMSG:
      return 1;
    case PalProtocol::IPTUX:
      return 2;
  }
  g_assert_not_reached();
  return 0;
}

PalProtocol ActiveToProtocol(gint active) {
  switch (active) {
    case 1:
      return PalProtocol::IPMSG;
    case 2:
      return PalProtocol::IPTUX;
    default:
      return PalProtocol::AUTO;
  }
}

}  // namespace

/**
 * 类构造函数.
 * @param pl
 */
RevisePal::RevisePal(Application* app, GtkWindow* parent, PalInfo* pl)
    : widset(NULL), mdlset(NULL), app(app), parent(parent), pal(pl) {
  InitSublayer();
}

/**
 * 类析构函数.
 */
RevisePal::~RevisePal() {
  ClearSublayer();
}

/**
 * 修正好友数据入口.
 * @param pal class PalInfo
 */
void RevisePal::ReviseEntryDo(Application* app,
                              GtkWindow* parent,
                              PalInfo* pal,
                              bool run) {
  RevisePal rpal(app, parent, pal);
  GtkWidget* dialog;

  /* 创建对话框 */
  dialog = rpal.CreateMainDialog();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                     rpal.CreateAllArea(), TRUE, TRUE, 0);
  rpal.SetAllValue();

  /* 运行对话框 */
  gtk_widget_show_all(dialog);
  if (run) {
    switch (gtk_dialog_run(GTK_DIALOG(dialog))) {
      case GTK_RESPONSE_OK:
        rpal.ApplyReviseData();
        break;
      default:
        break;
    }
  }
  gtk_widget_destroy(dialog);
}

/**
 * 初始化底层数据.
 */
void RevisePal::InitSublayer() {
  GtkTreeModel* model;

  g_datalist_init(&widset);
  g_datalist_init(&mdlset);

  model = CreateIconModel();
  g_datalist_set_data_full(&mdlset, "icon-model", model,
                           GDestroyNotify(g_object_unref));
  FillIconModel(model);
}

/**
 * 清空底层数据.
 */
void RevisePal::ClearSublayer() {
  g_datalist_clear(&widset);
  g_datalist_clear(&mdlset);
}

/**
 * 创建主对话框窗体.
 * @return 对话框
 */
GtkWidget* RevisePal::CreateMainDialog() {
  GtkWidget* dialog;

  dialog = gtk_dialog_new_with_buttons(
      _("Change Pal's Information"), parent, GTK_DIALOG_MODAL, _("_OK"),
      GTK_RESPONSE_OK, _("_Cancel"), GTK_RESPONSE_CANCEL, NULL);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
  gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
  gtk_container_set_border_width(GTK_CONTAINER(dialog), 5);
  gtk_widget_set_size_request(dialog, 400, -1);
  g_datalist_set_data(&widset, "dialog-widget", dialog);

  return dialog;
}

/**
 * 创建所有区域窗体.
 * @return 主窗体
 */
GtkWidget* RevisePal::CreateAllArea() {
  GtkWidget* grid;
  GtkWidget *label, *button, *widget;
  GtkTreeModel* model;

  grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 6);

  /* 好友昵称 */
  label = gtk_label_new(_("Pal's nickname:"));
  AttachLabel(GTK_GRID(grid), label, 0);
  widget = gtk_entry_new();
  g_object_set(widget, "has-tooltip", TRUE, NULL);
  gtk_widget_set_hexpand(widget, TRUE);
  gtk_grid_attach(GTK_GRID(grid), widget, 1, 0, 2, 1);
  g_signal_connect(widget, "query-tooltip", G_CALLBACK(entry_query_tooltip),
                   _("Please input pal's new nickname!"));
  g_datalist_set_data(&widset, "nickname-entry-widget", widget);

  /* 好友群组 */
  label = gtk_label_new(_("Pal's group name:"));
  AttachLabel(GTK_GRID(grid), label, 1);
  widget = gtk_entry_new();
  g_object_set(widget, "has-tooltip", TRUE, NULL);
  gtk_widget_set_hexpand(widget, TRUE);
  gtk_grid_attach(GTK_GRID(grid), widget, 1, 1, 2, 1);
  g_signal_connect(widget, "query-tooltip", G_CALLBACK(entry_query_tooltip),
                   _("Please input pal's new group name!"));
  g_datalist_set_data(&widset, "group-entry-widget", widget);

  /* 好友系统编码 */
  label = gtk_label_new(_("System coding:"));
  AttachLabel(GTK_GRID(grid), label, 2);
  widget = gtk_entry_new();
  g_object_set(widget, "has-tooltip", TRUE, NULL);
  gtk_widget_set_hexpand(widget, TRUE);
  gtk_grid_attach(GTK_GRID(grid), widget, 1, 2, 2, 1);
  g_signal_connect(widget, "query-tooltip", G_CALLBACK(entry_query_tooltip),
                   _("Be SURE to know what you are doing!"));
  g_datalist_set_data(&widset, "encode-entry-widget", widget);

  /* 好友头像 */
  label = gtk_label_new(_("Pal's face picture:"));
  AttachLabel(GTK_GRID(grid), label, 3);
  model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "icon-model"));
  widget = CreateIconTree(model);
  gtk_widget_set_hexpand(widget, TRUE);
  gtk_grid_attach(GTK_GRID(grid), widget, 1, 3, 1, 1);
  g_datalist_set_data(&widset, "icon-combo-widget", widget);
  button = gtk_button_new_with_label("...");
  gtk_grid_attach(GTK_GRID(grid), button, 2, 3, 1, 1);
  g_signal_connect(button, "clicked", G_CALLBACK(AddNewIcon), &widset);

  /* 协议 */
  label = gtk_label_new(_("Protocol:"));
  AttachLabel(GTK_GRID(grid), label, 4);
  widget = gtk_combo_box_text_new();
  AppendProtocolOption(GTK_COMBO_BOX_TEXT(widget), "AUTO");
  AppendProtocolOption(GTK_COMBO_BOX_TEXT(widget), "IPMSG");
  AppendProtocolOption(GTK_COMBO_BOX_TEXT(widget), "IPTUX");
  gtk_grid_attach(GTK_GRID(grid), widget, 1, 4, 2, 1);
  g_datalist_set_data(&widset, "protocol-combo-widget", widget);

  return grid;
}

/**
 * 给界面预设数据.
 */
void RevisePal::SetAllValue() {
  GtkWidget* widget;
  GtkTreeModel* model;
  gint active;

  /* 预置昵称 */
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "nickname-entry-widget"));
  gtk_entry_set_text(GTK_ENTRY(widget), pal->getName().c_str());
  /* 预置群组 */
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "group-entry-widget"));
  gtk_entry_set_text(GTK_ENTRY(widget), pal->getGroup().c_str());
  /* 预置编码 */
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "encode-entry-widget"));
  gtk_entry_set_text(GTK_ENTRY(widget), pal->getEncode().c_str());
  /* 预置头像 */
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "icon-combo-widget"));
  model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
  if (!pal->icon_file().empty()) {
    active = IconfileGetItemPos(model, pal->icon_file().c_str());
    gtk_combo_box_set_active(GTK_COMBO_BOX(widget), active);
  } else if (gtk_tree_model_iter_n_children(model, NULL) > 0) {
    gtk_combo_box_set_active(GTK_COMBO_BOX(widget), 0);
  }
  /* 预置协议 */
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "protocol-combo-widget"));
  gtk_combo_box_set_active(GTK_COMBO_BOX(widget),
                           ProtocolToActive(pal->protocol()));
}

/**
 * 应用修正后的数据.
 */
void RevisePal::ApplyReviseData() {
  GtkWidget* widget;
  GdkPixbuf* pixbuf;
  GtkTreeModel* model;
  GtkTreeIter iter;
  char path[MAX_PATHLEN];
  gchar *text, *file;
  const gchar* consttext;
  gint active;

  /* 获取昵称 */
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "nickname-entry-widget"));
  if (*(consttext = gtk_entry_get_text(GTK_ENTRY(widget))) != '\0') {
    pal->setName(consttext);
  }

  /* 获取群组 */
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "group-entry-widget"));
  if (*(consttext = gtk_entry_get_text(GTK_ENTRY(widget))) != '\0')
    pal->setGroup(consttext);
  else
    pal->setGroup("");

  /* 获取编码 */
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "encode-entry-widget"));
  text = gtk_editable_get_chars(GTK_EDITABLE(widget), 0, -1);
  g_strstrip(text);
  if (*text != '\0') {
    pal->setEncode(text);
  } else
    g_free(text);

  /* 获取头像 */
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "icon-combo-widget"));
  model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
  active = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
  if (active >= 0) {
    snprintf(path, MAX_PATHLEN, "%d", active);
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, 1, &file, -1);
    if (pal->icon_file() != file) {
      snprintf(path, MAX_PATHLEN, __PIXMAPS_PATH "/icon/%s", file);
      if (access(path, F_OK) != 0) {
        g_free(file);
        snprintf(path, MAX_PATHLEN, "%s" ICON_PATH "/%" PRIx32 ".png",
                 g_get_user_cache_dir(), ntohl(pal->ipv4().s_addr));
        pal->set_icon_file(
            stringFormat("%" PRIx32, (uint32_t)ntohl(pal->ipv4().s_addr)));
        gtk_tree_model_get(model, &iter, 0, &pixbuf, -1);
        gdk_pixbuf_save(pixbuf, path, "png", NULL, NULL);
        g_object_unref(pixbuf);
      } else {
        pal->set_icon_file(file);
      }
    } else {
      g_free(file);
    }
  }

  /* 获取协议 */
  widget = GTK_WIDGET(g_datalist_get_data(&widset, "protocol-combo-widget"));
  pal->setProtocol(
      ActiveToProtocol(gtk_combo_box_get_active(GTK_COMBO_BOX(widget))));

  /* 设置好友信息已被手工修改 */
  pal->setChanged(true);

  auto g_cthrd = app->getCoreThread();
  /* 更新好友信息 */
  g_cthrd->Lock();
  g_cthrd->UpdatePalToList(pal->ipv4());
  g_cthrd->Unlock();
}

/**
 * 头像树(icon-tree)底层数据结构.
 * 2,0 icon,1 iconfile \n
 * 头像;文件名(带后缀) \n
 * @return icon-model
 */
GtkTreeModel* RevisePal::CreateIconModel() {
  GtkListStore* model;

  model = gtk_list_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_STRING);

  return GTK_TREE_MODEL(model);
}

/**
 * 为头像树(icon-tree)填充底层数据.
 * @param model icon-model
 */
void RevisePal::FillIconModel(GtkTreeModel* model) {
  GtkIconTheme* theme;
  GdkPixbuf* pixbuf;
  GtkTreeIter iter;
  struct dirent* dirt;
  DIR* dir;
  char* file;

  theme = gtk_icon_theme_get_default();
  if ((dir = opendir(__PIXMAPS_PATH "/icon"))) {
    while ((dirt = readdir(dir))) {
      if (strcmp(dirt->d_name, ".") == 0 || strcmp(dirt->d_name, "..") == 0)
        continue;
      file = iptux_erase_filename_suffix(dirt->d_name);
      if ((pixbuf = gtk_icon_theme_load_icon(theme, file, MAX_ICONSIZE,
                                             GtkIconLookupFlags(0), NULL))) {
        gtk_list_store_append(GTK_LIST_STORE(model), &iter);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, pixbuf, 1,
                           dirt->d_name, -1);
        g_object_unref(pixbuf);
      }
      g_free(file);
    }
    closedir(dir);
  }
}

/**
 * 创建头像树(icon-tree).
 * @param model icon-model
 * @return 头像树
 */
GtkWidget* RevisePal::CreateIconTree(GtkTreeModel* model) {
  GtkWidget* combo;
  GtkCellRenderer* cell;

  combo = gtk_combo_box_new_with_model(model);
  gtk_combo_box_set_wrap_width(GTK_COMBO_BOX(combo), 5);
  cell = gtk_cell_renderer_pixbuf_new();
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), cell, FALSE);
  gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), cell, "pixbuf", 0,
                                 NULL);

  return combo;
}

/**
 * 查询(pathname)文件在(model)中的位置，若没有则加入到后面.
 * @param model model
 * @param pathname 文件路径
 * @return 位置
 */
gint RevisePal::IconfileGetItemPos(GtkTreeModel* model, const char* pathname) {
  GtkIconTheme* theme;
  GdkPixbuf* pixbuf;
  GtkTreeIter iter;
  const char* ptr;
  gchar* file;
  gint result, pos;

  /* 让ptr指向文件名 */
  ptr = strrchr(pathname, '/');
  ptr = ptr ? ptr + 1 : pathname;
  /* 查询model中是否已经存在此文件 */
  pos = 0;
  if (gtk_tree_model_get_iter_first(model, &iter)) {
    do {
      gtk_tree_model_get(model, &iter, 1, &file, -1);
      result = strcmp(ptr, file);
      g_free(file);
      if (result == 0)
        return pos;
      pos++;
    } while (gtk_tree_model_iter_next(model, &iter));
  }
  /* 将文件加入model */
  if (access(pathname, F_OK) != 0) {
    theme = gtk_icon_theme_get_default();
    file = iptux_erase_filename_suffix(pathname);
    pixbuf = gtk_icon_theme_load_icon(theme, file, MAX_ICONSIZE,
                                      GtkIconLookupFlags(0), NULL);
    g_free(file);
  } else
    pixbuf = gdk_pixbuf_new_from_file_at_size(pathname, MAX_ICONSIZE,
                                              MAX_ICONSIZE, NULL);
  if (pixbuf) {
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, pixbuf, 1, ptr, -1);
    g_object_unref(pixbuf);
  } else
    pos = -1;

  return pos;
}

/**
 * 添加新的头像数据.
 * @param button button
 * @param widset widget set
 */
void RevisePal::AddNewIcon(GtkWidget* button, GData** widset) {
  GtkWidget *parent, *combo;
  GtkTreeModel* model;
  gchar* filename;
  gint active;

  parent = GTK_WIDGET(g_datalist_get_data(widset, "dialog-widget"));
  if (!(filename = choose_file_with_preview(_("Please select a face picture"),
                                            parent)))
    return;

  combo = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "icon-combo-widget"));
  model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo));
  active = IconfileGetItemPos(model, filename);
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo), active);
  g_free(filename);
}

}  // namespace iptux
