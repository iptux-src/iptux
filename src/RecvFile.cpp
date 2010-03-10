//
// C++ Implementation: RecvFile
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "RecvFile.h"
#include "RecvFileData.h"
#include "ProgramData.h"
#include "callback.h"
#include "utils.h"
extern ProgramData progdt;

/**
 * 类构造函数.
 */
RecvFile::RecvFile():widset(NULL), mdlset(NULL), dtset(NULL), filelist(NULL)
{
        InitSublayer();
        ReadUILayout();
}

/**
 * 类析构函数.
 */
RecvFile::~RecvFile()
{
        WriteUILayout();
        ClearSublayer();
}

/**
 * 文件接受入口.
 * @param para 文件参数
 */
void RecvFile::RecvEntry(GData *para)
{
        RecvFile *rfile;
        GtkWidget *window;

        rfile = new RecvFile;
        rfile->ParseFilePara(&para);
        gdk_threads_enter();
        window = rfile->CreateWindow(&para);
        gtk_container_add(GTK_CONTAINER(window), rfile->CreateAllArea());
        gtk_widget_show_all(window);
        gdk_threads_leave();
        g_datalist_clear(&para);        //para提供的数据已经没用了，秒掉它
//      delete rfile;//请不要这样做，类只能在主窗口被摧毁后才能释放
}

/**
 * 初始化底层数据.
 */
void RecvFile::InitSublayer()
{
        GtkTreeModel *model;

        g_datalist_init(&widset);
        g_datalist_init(&mdlset);
        g_datalist_init(&dtset);

        model = CreateFileModel();
        g_datalist_set_data_full(&mdlset, "file-model", model,
                                 GDestroyNotify(g_object_unref));
}

/**
 * 清空底层数据.
 */
void RecvFile::ClearSublayer()
{
        g_datalist_clear(&widset);
        g_datalist_clear(&mdlset);
        g_datalist_clear(&dtset);
        for (GSList *tlist = filelist; tlist; tlist = g_slist_next(tlist))
                delete (FileInfo *)tlist->data;
        g_slist_free(filelist);
}

/**
 * 读取对话框的UI布局数据.
 */
void RecvFile::ReadUILayout()
{
        GConfClient *client;
        gint numeric;

        client = gconf_client_get_default();

        numeric = gconf_client_get_int(client, GCONF_PATH "/recv_window_width", NULL);
        numeric = numeric ? numeric : 500;
        g_datalist_set_data(&dtset, "window-width", GINT_TO_POINTER(numeric));
        numeric = gconf_client_get_int(client, GCONF_PATH "/recv_window_height", NULL);
        numeric = numeric ? numeric : 350;
        g_datalist_set_data(&dtset, "window-height", GINT_TO_POINTER(numeric));

        g_object_unref(client);
}

/**
 * 写出对话框的UI布局数据.
 */
void RecvFile::WriteUILayout()
{
        GConfClient *client;
        gint numeric;

        client = gconf_client_get_default();

        numeric = GPOINTER_TO_INT(g_datalist_get_data(&dtset, "window-width"));
        gconf_client_set_int(client, GCONF_PATH "/recv_window_width", numeric, NULL);
        numeric = GPOINTER_TO_INT(g_datalist_get_data(&dtset, "window-height"));
        gconf_client_set_int(client, GCONF_PATH "/recv_window_height", numeric, NULL);

        g_object_unref(client);
}

/**
 * 分析文件参数.
 * @param para 文件参数
 */
void RecvFile::ParseFilePara(GData **para)
{
        PalInfo *pal;
        FileInfo *file;
        char *extra;
        uint32_t packetn;

        pal = (PalInfo *)g_datalist_get_data(para, "palinfo");
        extra = (char *)g_datalist_get_data(para, "extra-data");
        packetn = GPOINTER_TO_UINT(g_datalist_get_data(para, "packetno"));

        while (extra && *extra) {
                file = DivideFileinfo(&extra);
                file->packetn = packetn;
                file->fileown = pal;
                filelist = g_slist_append(filelist, file);
        }
}

/**
 * 从文件信息串中分离出文件信息数据.
 * @param extra 文件信息串
 * @return 文件信息数据
 */
FileInfo *RecvFile::DivideFileinfo(char **extra)
{
        FileInfo *file;

        file = new FileInfo;
        file->fileid = iptux_get_dec_number(*extra, ':', 0);
        file->fileattr = iptux_get_hex_number(*extra, ':', 4);
        file->filesize = iptux_get_hex64_number(*extra, ':', 2);
        file->filepath = ipmsg_get_filename(*extra, ':', 1);

        //分割，格式1(\a) 格式2(:\a) 格式3(\a:) 格式4(:\a:)
        *extra = strchr(*extra, '\a');
        if (*extra)     //跳过'\a'字符
                (*extra)++;
        if (*extra && (**extra == ':')) //跳过可能存在的':'字符
                (*extra)++;

        return file;
}

/**
 * 创建主对话框窗口.
 * @param para 文件参数
 * @return 窗口
 */
GtkWidget *RecvFile::CreateWindow(GData **para)
{
        GtkWidget *window;
        gint width, height;
        uint32_t commandn;

        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        commandn = GPOINTER_TO_UINT(g_datalist_get_data(para, "commandno"));
        if (commandn & IPTUX_SHAREDOPT)
                gtk_window_set_title(GTK_WINDOW(window), _("Pal's Shared Resources"));
        else
                gtk_window_set_title(GTK_WINDOW(window), _("Files Receive Management"));
        width = GPOINTER_TO_INT(g_datalist_get_data(&dtset, "window-width"));
        height = GPOINTER_TO_INT(g_datalist_get_data(&dtset, "window-height"));
        gtk_window_set_default_size(GTK_WINDOW(window), width, height);
        gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
        gtk_container_set_border_width(GTK_CONTAINER(window), 5);
        g_signal_connect_swapped(window, "destroy", G_CALLBACK(DialogDestroy), this);
        g_signal_connect(window, "configure-event",
                         G_CALLBACK(WindowConfigureEvent), &dtset);
        g_datalist_set_data(&widset, "window-widget", window);

        return window;
}

/**
 * 创建所有区域.
 * @return 主窗体
 */
GtkWidget *RecvFile::CreateAllArea()
{
        GtkWidget *box, *sw;
        GtkWidget *hbox, *label;
        GtkWidget *button, *widget, *window;
        GtkTreeModel *model;
        GtkTreeSelection *selection;

        box = gtk_vbox_new(FALSE, 0);

        /* 创建文件树区域 */
        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                                 GTK_SHADOW_ETCHED_IN);
        gtk_box_pack_start(GTK_BOX(box), sw, TRUE, TRUE, 0);
        model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "file-model"));
        FillFileModel(model);
        widget = CreateFileTree(model);
        gtk_container_add(GTK_CONTAINER(sw), widget);
        g_signal_connect(widget, "button-press-event", G_CALLBACK(PopupPickMenu), NULL);
        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
        g_signal_connect(selection, "changed",
                         G_CALLBACK(FiletreeSelectItemChanged), &widset);
        g_datalist_set_data(&widset, "file-treeview-widget", widget);

        /* 创建路径重定位区域 */
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
        label = gtk_label_new(_("Save files to: "));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        widget = CreateArchiveChooser();
        gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
        g_signal_connect(widget, "current-folder-changed",
                         G_CALLBACK(ChooserResetStatelabel), &widset);
        g_signal_connect(widget, "current-folder-changed",
                         G_CALLBACK(ChooserResetFiletree), &widset);
        g_datalist_set_data(&widset, "file-chooser-widget", widget);
        label = gtk_label_new(NULL);
        gtk_box_pack_end(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        g_datalist_set_data(&widset, "state-label-widget", label);

        /* 偶是可爱的窗体分割线 */
        widget = gtk_hseparator_new();
        gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);

        /* 拒绝、接受按钮区域 */
        window = GTK_WIDGET(g_datalist_get_data(&widset, "window-widget"));
        hbox = gtk_hbutton_box_new();
        gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox), GTK_BUTTONBOX_END);
        gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
        button = gtk_button_new_with_label(_("Accept"));
        gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
        g_signal_connect_swapped(button, "clicked",
                         G_CALLBACK(AttachRecvFile), &widset);
        g_signal_connect_swapped(button, "clicked",
                         G_CALLBACK(gtk_widget_destroy), window);
        button = gtk_button_new_with_label(_("Cancel"));
        gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
        g_signal_connect_swapped(button, "clicked",
                         G_CALLBACK(gtk_widget_destroy), window);

        return box;
}

/**
 * 创建文件树(file-tree)底层数据结构.
 * 7,0 toggle,1 owner,2 filename,3 size,4 type,5 path,6 data \n
 * 是否被选中;文件来源;文件名;文件大小;文件类型;保存路径;链表项指针 \n
 * @return file-model
 */
GtkTreeModel *RecvFile::CreateFileModel()
{
        GtkListStore *model;

        model = gtk_list_store_new(7, G_TYPE_BOOLEAN, G_TYPE_STRING,
                         G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                         G_TYPE_STRING,  G_TYPE_POINTER);

        return GTK_TREE_MODEL(model);
}

/**
 * 为文件树(file-tree)填充底层数据.
 * @param model file-model
 */
void RecvFile::FillFileModel(GtkTreeModel *model)
{
        GtkTreeIter iter;
        GSList *tlist;
        FileInfo *file;
        gchar *filesize;
        const gchar *filetype;

        tlist = filelist;
        while (tlist) {
                file = (FileInfo *)tlist->data;
                filesize = numeric_to_size(file->filesize);
                switch (GET_MODE(file->fileattr)) {
                case IPMSG_FILE_REGULAR:
                        filetype = _("regular");
                        break;
                case IPMSG_FILE_DIR:
                        filetype = _("directory");
                        break;
                default:
                        filetype = _("unknown");
                        break;
                }
                gtk_list_store_append(GTK_LIST_STORE(model), &iter);
                gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, TRUE,
                                 1, file->fileown->name, 2, file->filepath,
                                 3, filesize, 4, filetype, 5, progdt.path,
                                 6, tlist, -1);
                g_free(filesize);
                tlist = g_slist_next(tlist);
        }
}

/**
 * 创建文件树(file-tree).
 * @param model file-model
 * @return 文件树
 */
GtkWidget *RecvFile::CreateFileTree(GtkTreeModel *model)
{
        GtkWidget *view;
        GtkTreeViewColumn *column;
        GtkCellRenderer *cell;
        GtkTreeSelection *selection;

        view = gtk_tree_view_new_with_model(model);
        gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), TRUE);
        gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(view), TRUE);
        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
        gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

        cell = gtk_cell_renderer_toggle_new();
        column = gtk_tree_view_column_new_with_attributes(_("Recv"), cell,
                                                         "active", 0, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
        g_signal_connect_swapped(cell, "toggled", G_CALLBACK(model_turn_select), model);

        cell = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(_("Source"), cell,
                                                         "text", 1, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

        cell = gtk_cell_renderer_text_new();
        g_object_set(cell, "editable", TRUE, NULL);
        g_object_set_data(G_OBJECT(cell), "column-number", GINT_TO_POINTER(2));
        column = gtk_tree_view_column_new_with_attributes(_("Filename"), cell,
                                                         "text", 2, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
        g_signal_connect(cell, "edited", G_CALLBACK(CellEditText), model);

        cell = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(_("Size"), cell,
                                                         "text", 3, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

        cell = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(_("Type"), cell,
                                                         "text", 4, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

        return view;
}

/**
 * 创建文件选择器.
 * @return 选择器
 */
GtkWidget *RecvFile::CreateArchiveChooser()
{
        GtkWidget *chooser;

        chooser = gtk_file_chooser_button_new(_("Please select download folder"),
                                         GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
        gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(chooser), FALSE);
        gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(chooser), TRUE);
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), progdt.path);

        return chooser;
}

/**
 * 创建弹出菜单.
 * @param model model
 * @return 菜单
 */
GtkWidget *RecvFile::CreatePopupMenu(GtkTreeModel *model)
{
        GtkWidget *menu, *menuitem;

        menu = gtk_menu_new();

        menuitem = gtk_menu_item_new_with_label(_("Select All"));
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
        g_signal_connect_swapped(menuitem, "activate",
                         G_CALLBACK(model_select_all), model);

        menuitem = gtk_menu_item_new_with_label(_("Reverse Select"));
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
        g_signal_connect_swapped(menuitem, "activate",
                         G_CALLBACK(model_turn_all), model);

        menuitem = gtk_menu_item_new_with_label(_("Clear Up"));
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
        g_signal_connect_swapped(menuitem, "activate",
                         G_CALLBACK(model_clear_all), model);

        return menu;
}

/**
 * 弹出选择菜单.
 * @param treeview text-view
 * @param event event
 * @return Gtk+库所需
 */
gboolean RecvFile::PopupPickMenu(GtkWidget *treeview, GdkEventButton *event)
{
        GtkWidget *menu;
        GtkTreeModel *model;

        if (event->button != 3)
                return FALSE;
        model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
        menu = CreatePopupMenu(model);
        gtk_widget_show_all(menu);
        gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                                 event->button, event->time);

        return TRUE;
}

/**
 * 重设model->cell的数据.
 * @param renderer cell-renderer-text
 * @param path path
 * @param newtext newtext
 * @param model model
 */
void RecvFile::CellEditText(GtkCellRendererText *renderer, gchar *path,
                                 gchar *newtext, GtkTreeModel *model)
{
        GtkTreeIter iter;
        gint column;

        column = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(renderer), "column-number"));
        gtk_tree_model_get_iter_from_string(model, &iter, path);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, column, newtext, -1);
}

/**
 * 文件树(file-tree)所选项变更的响应处理函数.
 * @param selection tree-selection
 * @param widset widget set
 */
void RecvFile::FiletreeSelectItemChanged(GtkTreeSelection *selection, GData **widset)
{
        GtkWidget *widget;
        GtkTreeModel *model;
        GtkTreeIter iter;
        gchar *path;

        widget = GTK_WIDGET(g_datalist_get_data(widset, "file-chooser-widget"));
        if (!gtk_tree_selection_get_selected(selection, &model, &iter))
                return;
        gtk_tree_model_get(model, &iter, 5, &path, -1);
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(widget), path);
        g_free(path);
}

/**
 * 文件选择器所选择文件变更的响应处理函数.
 * @param chooser file-chooser
 * @param widset widget set
 */
void RecvFile::ChooserResetStatelabel(GtkWidget *chooser, GData **widset)
{
        GtkWidget *widget;
        char buf[MAX_BUFLEN];
        char *path, *str_avail, *str_total;
        int64_t avail, total;

        widget = GTK_WIDGET(g_datalist_get_data(widset, "state-label-widget"));
        path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
        get_file_system_info(path, &avail, &total);
        g_free(path);
        str_avail = numeric_to_size(avail);
        str_total = numeric_to_size(total);
        snprintf(buf, MAX_BUFLEN, _("Free:%s Total:%s"), str_avail, str_total);
        g_free(str_avail);
        g_free(str_total);
        gtk_label_set_label(GTK_LABEL(widget), buf);
}

/**
 * 文件选择器所选择文件变更的响应处理函数.
 * @param chooser file-chooser
 * @param widset widget set
 */
void RecvFile::ChooserResetFiletree(GtkWidget *chooser, GData **widset)
{
        GtkWidget *widget;
        GtkTreeSelection *selection;
        GtkTreeModel *model;
        GtkTreeIter iter;
        gchar *path;

        widget = GTK_WIDGET(g_datalist_get_data(widset, "file-treeview-widget"));
        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
        model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
        path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
        if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
                /* 如果没有项被选中则改变所有的项 */
                if (gtk_tree_model_get_iter_first(model, &iter)) {
                        do {
                                gtk_list_store_set(GTK_LIST_STORE(model),
                                                         &iter, 5, path, -1);
                        }  while (gtk_tree_model_iter_next(model, &iter));
                }
        } else
                gtk_list_store_set(GTK_LIST_STORE(model), &iter, 5, path, -1);
        g_free(path);
}

/**
 * 文件接收消息响应处理函数.
 * @param widset widget set
 */
void RecvFile::AttachRecvFile(GData **widset)
{
        GtkWidget *widget;
        GtkTreeModel *model;
        GtkTreeIter iter;
        gchar *filename, *filepath;
        FileInfo *file;
        GSList *tlist;
        pthread_t pid;
        gboolean active;

        /* 考察数据集中是否存在项 */
        widget = GTK_WIDGET(g_datalist_get_data(widset, "file-treeview-widget"));
        model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
        if (!gtk_tree_model_get_iter_first(model, &iter))
                return;

        /* 将选中的项投入文件数据接收类 */
        do {
                gtk_tree_model_get(model, &iter, 0, &active, 2, &filename,
                                                 5, &filepath, 6, &tlist, -1);
                if (active) {
                        file = (FileInfo *)tlist->data;
                        tlist->data = NULL;
                        g_free(file->filepath);
                        file->filepath = g_strdup_printf("%s%s%s", filepath,
                                         *(filepath + 1) != '\0' ? "/" : "",
                                         filename);
                        pthread_create(&pid, NULL, ThreadFunc(ThreadRecvFile), file);
                        pthread_detach(pid);
                }
                g_free(filename);
                g_free(filepath);
        } while (gtk_tree_model_iter_next(model, &iter));
}

/**
 * 文件接收窗口位置&大小改变的响应处理函数.
 * @param window 主窗口
 * @param event the GdkEventConfigure which triggered this signal
 * @param dtset data set
 * @return Gtk+库所需
 */
gboolean RecvFile::WindowConfigureEvent(GtkWidget *window,
                                 GdkEventConfigure *event, GData **dtset)
{
        g_datalist_set_data(dtset, "window-width", GINT_TO_POINTER(event->width));
        g_datalist_set_data(dtset, "window-height", GINT_TO_POINTER(event->height));

        return FALSE;
}

/**
 * 主窗口被摧毁的响应处理函数.
 * @param rfile 接受文件类
 */
void RecvFile::DialogDestroy(RecvFile *rfile)
{
        delete rfile;
}

/**
 * 接收文件数据.
 * @param file 文件信息
 */
void RecvFile::ThreadRecvFile(FileInfo *file)
{
        RecvFileData rfdt(file);

        rfdt.RecvFileDataEntry();
        delete file;
}
