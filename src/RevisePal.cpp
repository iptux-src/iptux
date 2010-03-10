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
#include "RevisePal.h"
#include "CoreThread.h"
#include "MainWindow.h"
#include "callback.h"
#include "utils.h"
extern CoreThread cthrd;
extern MainWindow mwin;

/**
 * 类构造函数.
 * @param pl
 */
RevisePal::RevisePal(PalInfo *pl):widset(NULL), mdlset(NULL), pal(pl)
{
        InitSublayer();
}

/**
 * 类析构函数.
 */
RevisePal::~RevisePal()
{
        ClearSublayer();
}

/**
 * 修正好友数据入口.
 * @param pal class PalInfo
 */
void RevisePal::ReviseEntry(PalInfo *pal)
{
        RevisePal rpal(pal);
        GtkWidget *dialog;

        /* 创建对话框 */
        dialog = rpal.CreateMainDialog();
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
                         rpal.CreateAllArea(), TRUE, TRUE, 0);
        rpal.SetAllValue();

        /* 运行对话框 */
        gtk_widget_show_all(dialog);
        switch (gtk_dialog_run(GTK_DIALOG(dialog))) {
        case GTK_RESPONSE_OK:
                rpal.ApplyReviseData();
                break;
        default:
                break;
        }
        gtk_widget_destroy(dialog);
}

/**
 * 初始化底层数据.
 */
void RevisePal::InitSublayer()
{
        GtkTreeModel *model;

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
void RevisePal::ClearSublayer()
{
        g_datalist_clear(&widset);
        g_datalist_clear(&mdlset);
}

/**
 * 创建主对话框窗体.
 * @return 对话框
 */
GtkWidget *RevisePal::CreateMainDialog()
{
        GtkWidget *dialog;

        dialog = gtk_dialog_new_with_buttons(_("Change Pal's Information"),
                         GTK_WINDOW(mwin.ObtainWindow()), GTK_DIALOG_MODAL,
                         GTK_STOCK_OK, GTK_RESPONSE_OK,
                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
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
GtkWidget *RevisePal::CreateAllArea()
{
        GtkWidget *box, *hbox;
        GtkWidget *label, *button, *widget;
        GtkTreeModel *model;

        box = gtk_vbox_new(FALSE, 0);

        /* 好友昵称 */
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
        label = gtk_label_new(_("Pal's nickname:"));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        widget = gtk_entry_new();
        g_object_set(widget, "has-tooltip", TRUE, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
        g_signal_connect(widget, "query-tooltip", G_CALLBACK(entry_query_tooltip),
                                         _("Please input pal's new nickname!"));
        g_datalist_set_data(&widset, "nickname-entry-widget", widget);

        /* 好友群组 */
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
        label = gtk_label_new(_("Pal's group name:"));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        widget = gtk_entry_new();
        g_object_set(widget, "has-tooltip", TRUE, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
        g_signal_connect(widget, "query-tooltip", G_CALLBACK(entry_query_tooltip),
                                         _("Please input pal's new group name!"));
        g_datalist_set_data(&widset, "group-entry-widget", widget);

        /* 好友系统编码 */
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
        label = gtk_label_new(_("System coding:"));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        widget = gtk_entry_new();
        g_object_set(widget, "has-tooltip", TRUE, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
        g_signal_connect(widget, "query-tooltip", G_CALLBACK(entry_query_tooltip),
                                 _("Be SURE to know what you are doing!"));
        g_datalist_set_data(&widset, "encode-entry-widget", widget);

        /* 好友头像 */
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
        label = gtk_label_new(_("Pal's face picture:"));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "icon-model"));
        widget = CreateIconTree(model);
        gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
        g_datalist_set_data(&widset, "icon-combo-widget", widget);
        button = gtk_button_new_with_label("...");
        gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
        g_signal_connect(button, "clicked", G_CALLBACK(AddNewIcon), &widset);

        /* 协议兼容性 */
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
        widget = gtk_check_button_new_with_label(
                         _("Be compatible with iptux's protocol (DANGEROUS)"));
        gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
        g_datalist_set_data(&widset, "compatible-check-widget", widget);

        return box;
}

/**
 * 给界面预设数据.
 */
void RevisePal::SetAllValue()
{
        GtkWidget *widget;
        GtkTreeModel *model;
        gint active;

        /* 预置昵称 */
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "nickname-entry-widget"));
        gtk_entry_set_text(GTK_ENTRY(widget), pal->name);
        /* 预置群组 */
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "group-entry-widget"));
        if (pal->group)
                gtk_entry_set_text(GTK_ENTRY(widget), pal->group);
        /* 预置编码 */
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "encode-entry-widget"));
        gtk_entry_set_text(GTK_ENTRY(widget), pal->encode);
        /* 预置头像 */
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "icon-combo-widget"));
        model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
        active = IconfileGetItemPos(model, pal->iconfile);
        gtk_combo_box_set_active(GTK_COMBO_BOX(widget), active);
        /* 预置兼容性 */
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "compatible-check-widget"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
                                 FLAG_ISSET(pal->flags, 0));
}

/**
 * 应用修正后的数据.
 */
void RevisePal::ApplyReviseData()
{
        GtkWidget *widget;
        GdkPixbuf *pixbuf;
        GtkTreeModel *model;
        GtkTreeIter iter;
        char path[MAX_PATHLEN];
        gchar *text, *file;
        const gchar *consttext;
        gint active;

        /* 获取昵称 */
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "nickname-entry-widget"));
        if (*(consttext = gtk_entry_get_text(GTK_ENTRY(widget))) != '\0') {
                g_free(pal->name);
                pal->name = g_strdup(consttext);
        }

        /* 获取群组 */
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "group-entry-widget"));
        g_free(pal->group);
        if (*(consttext = gtk_entry_get_text(GTK_ENTRY(widget))) != '\0')
                pal->group = g_strdup(consttext);
        else
                pal->group = NULL;

        /* 获取编码 */
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "encode-entry-widget"));
        text = gtk_editable_get_chars(GTK_EDITABLE(widget), 0, -1);
        g_strstrip(text);
        if (*text != '\0') {
                g_free(pal->encode);
                pal->encode = text;
        } else
                g_free(text);

        /* 获取头像 */
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "icon-combo-widget"));
        model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
        active = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
        snprintf(path, MAX_PATHLEN, "%d", active);
        gtk_tree_model_get_iter_from_string(model, &iter, path);
        gtk_tree_model_get(model, &iter, 1, &file, -1);
        if (strcmp(pal->iconfile, file) != 0) {
                snprintf(path, MAX_PATHLEN, __PIXMAPS_PATH "/icon/%s", file);
                if (access(path, F_OK) != 0) {
                        g_free(file);
                        g_free(pal->iconfile);
                        snprintf(path, MAX_PATHLEN, "%s" ICON_PATH "/%" PRIx32,
                                         g_get_user_cache_dir(), pal->ipv4);
                        pal->iconfile = g_strdup_printf("%" PRIx32, pal->ipv4);
                        gtk_tree_model_get(model, &iter, 0, &pixbuf, -1);
                        gdk_pixbuf_save(pixbuf, path, "png", NULL, NULL);
                        gtk_icon_theme_add_builtin_icon(pal->iconfile,
                                                 MAX_ICONSIZE, pixbuf);
                        g_object_unref(pixbuf);
                } else {
                        g_free(pal->iconfile);
                        pal->iconfile = file;
                }
        } else
                g_free(file);

        /* 获取兼容性 */
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "compatible-check-widget"));
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
                FLAG_SET(pal->flags, 0);
        else
                FLAG_CLR(pal->flags, 0);

        /* 设置好友信息已被手工修改 */
        FLAG_SET(pal->flags, 2);

        /* 更新好友信息 */
        pthread_mutex_lock(cthrd.GetMutex());
        cthrd.UpdatePalToList(pal->ipv4);
        pthread_mutex_unlock(cthrd.GetMutex());
        mwin.UpdateItemToPaltree(pal->ipv4);
}

/**
 * 头像树(icon-tree)底层数据结构.
 * 2,0 icon,1 iconfile \n
 * 头像;文件名(带后缀) \n
 * @return icon-model
 */
GtkTreeModel *RevisePal::CreateIconModel()
{
        GtkListStore *model;

        model = gtk_list_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_STRING);

        return GTK_TREE_MODEL(model);
}

/**
 * 为头像树(icon-tree)填充底层数据.
 * @param model icon-model
 */
void RevisePal::FillIconModel(GtkTreeModel *model)
{
        GtkIconTheme *theme;
        GdkPixbuf *pixbuf;
        GtkTreeIter iter;
        struct dirent *dirt;
        DIR *dir;
        char *file;

        theme = gtk_icon_theme_get_default();
        if ( (dir = opendir(__PIXMAPS_PATH "/icon"))) {
                while ( (dirt = readdir(dir))) {
                        if (strcmp(dirt->d_name, ".") == 0
                                 || strcmp(dirt->d_name, "..") == 0)
                                continue;
                        file = iptux_erase_filename_suffix(dirt->d_name);
                        if ( (pixbuf = gtk_icon_theme_load_icon(theme, file, MAX_ICONSIZE,
                                                         GtkIconLookupFlags(0), NULL))) {
                                gtk_list_store_append(GTK_LIST_STORE(model), &iter);
                                gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                                                 0, pixbuf, 1, dirt->d_name, -1);
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
GtkWidget *RevisePal::CreateIconTree(GtkTreeModel *model)
{
        GtkWidget *combo;
        GtkCellRenderer *cell;

        combo = gtk_combo_box_new_with_model(model);
        gtk_combo_box_set_wrap_width(GTK_COMBO_BOX(combo), 5);
        cell = gtk_cell_renderer_pixbuf_new();
        gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), cell, FALSE);
        gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), cell, "pixbuf", 0, NULL);

        return combo;
}

/**
 * 查询(pathname)文件在(model)中的位置，若没有则加入到后面.
 * @param model model
 * @param pathname 文件路径
 * @return 位置
 */
gint RevisePal::IconfileGetItemPos(GtkTreeModel *model, const char *pathname)
{
        GtkIconTheme *theme;
        GdkPixbuf *pixbuf;
        GtkTreeIter iter;
        const char *ptr;
        gchar *file;
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
                pixbuf = gdk_pixbuf_new_from_file_at_size(pathname,
                                 MAX_ICONSIZE, MAX_ICONSIZE, NULL);
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
void RevisePal::AddNewIcon(GtkWidget *button, GData **widset)
{
        GtkWidget *parent, *combo;
        GtkTreeModel *model;
        gchar *filename;
        gint active;

        parent = GTK_WIDGET(g_datalist_get_data(widset, "dialog-widget"));
        if (!(filename = choose_file_with_preview(
                 _("Please select a face picture"), parent)))
                return;

        combo = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "icon-combo-widget"));
        model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo));
        active = IconfileGetItemPos(model, filename);
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo), active);
        g_free(filename);
}
