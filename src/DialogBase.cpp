//
// C++ Implementation: DialogBase
//
// Description:
// 这个类是DialogPeer和DialogGroup的相同部分。尽量把相同的部分放在一起。
//
// Author: Jiejing.Zhang <kzjeef@gmail.com>, (C) 2010
//         Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "DialogBase.h"
#include "DialogPeer.h"
#include "ProgramData.h"
#include "CoreThread.h"
#include "MainWindow.h"
#include "LogSystem.h"
#include "Command.h"
#include "SendFile.h"
#include "HelpDialog.h"
#include "callback.h"
#include "output.h"
#include "support.h"
#include "utils.h"
#include "AnalogFS.h"

extern ProgramData progdt;
extern CoreThread cthrd;
extern MainWindow mwin;

DialogBase::DialogBase(GroupInfo *grp)
        :widset(NULL), mdlset(NULL),dtset(NULL), accel(NULL), grpinf(grp),
         totalsendsize(0)
{
        InitSublayerGeneral();
}

DialogBase::~DialogBase()
{
        if(timersend > 0)
            g_source_remove(timersend);
        ClearSublayerGeneral();
}

/**
 * 初始化底层数据.
 */
void DialogBase::InitSublayerGeneral()
{
        g_datalist_init(&widset);
        g_datalist_init(&mdlset);
        g_datalist_init(&dtset);
        accel = gtk_accel_group_new();
}

/**
 * 清空底层数据.
 */
void DialogBase::ClearSublayerGeneral()
{
        if (FLAG_ISSET(progdt.flags, 3))
                ClearHistoryTextView();
        grpinf->dialog = NULL;
        g_datalist_clear(&widset);
        g_datalist_clear(&mdlset);
        g_datalist_clear(&dtset);
        g_object_unref(accel);
}

/**
 * 清空聊天历史记录.
 */
void DialogBase::ClearHistoryTextView()
{
        GtkWidget *widget;
        GtkTextBuffer *buffer;
        GtkTextTagTable *table;
        GtkTextIter start, end;
        GSList *taglist, *tlist;

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "history-textview-widget"));
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
        table = gtk_text_buffer_get_tag_table(buffer);

        /* 清除用于局部标记的GtkTextTag */
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        while (!gtk_text_iter_equal(&start, &end)) {
                tlist = taglist = gtk_text_iter_get_tags(&start);
                while (tlist) {
                        /* 如果没有"global"标记，则表明此tag为局部标记，可以移除 */
                        if (!g_object_get_data(G_OBJECT(tlist->data), "global"))
                                gtk_text_tag_table_remove(table,
                                         GTK_TEXT_TAG(tlist->data));
                        tlist = g_slist_next(tlist);
                }
                g_slist_free(taglist);
                gtk_text_iter_forward_char(&start);
        }

        /* 清除内容 */
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        gtk_text_buffer_delete(buffer, &start, &end);
}

/**
 * 滚动聊天历史记录区.
 */
void DialogBase::ScrollHistoryTextview()
{
        GtkWidget *widget;
        GtkTextBuffer *buffer;
        GtkTextIter end;
        GtkTextMark *mark;

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "history-textview-widget"));
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
        gtk_text_buffer_get_end_iter(buffer, &end);
        mark = gtk_text_buffer_create_mark(buffer, NULL, &end, FALSE);
        gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(widget), mark, 0.0, TRUE, 0.0, 0.0);
        gtk_text_buffer_delete_mark(buffer, mark);
}

/**
 * 窗口打开情况下，新消息来到以后的接口
 */
void DialogBase::OnNewMessageComing()
{
    this->NotifyUser();
    this->ScrollHistoryTextview();
}

/**
 * 在窗口打开并且没有设置为最顶端的窗口时候，用窗口在任务栏的闪动来提示用户
 */
void DialogBase::NotifyUser()
{
#if GTK_CHECK_VERSION(2,8,0)
    GtkWindow *window;
    window = GTK_WINDOW(g_datalist_get_data(&widset, "window-widget"));
    if (!gtk_window_has_toplevel_focus(window))
        gtk_window_set_urgency_hint(window, TRUE);
#endif
}


/**
 * 添加附件.
 * @param list 文件链表
 */
void DialogBase::AttachEnclosure(const GSList *list)
{
        GtkWidget *widget,*pbar;
        GtkTreeModel *model;
        GtkTreeIter iter;
        GdkPixbuf *pixbuf, *rpixbuf, *dpixbuf;
        struct stat64 st;
        const GSList *tlist,*pallist;
        AnalogFS afs;
        int64_t filesize;
        char *filename,*filepath, *progresstip;
        FileInfo *file;
        uint32_t filenum = 0;

        /* 获取文件图标 */
        rpixbuf = obtain_pixbuf_from_stock(GTK_STOCK_FILE);
        dpixbuf = obtain_pixbuf_from_stock(GTK_STOCK_DIRECTORY);

        /* 插入附件树 */
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "file-send-treeview-widget"));
        model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
        tlist = list;
        while (tlist) {
            if (stat64((const char *)tlist->data, &st) == -1
                     || !(S_ISREG(st.st_mode) || S_ISDIR(st.st_mode))) {
                    tlist = g_slist_next(tlist);
                    continue;
            }
            /* 获取文件类型图标 */
            if (S_ISREG(st.st_mode))
                    pixbuf = rpixbuf;
            else if (S_ISDIR(st.st_mode))
                    pixbuf = dpixbuf;
            else
                    pixbuf = NULL;
            filesize = afs.ftwsize((char *)tlist->data);
            filename = ipmsg_get_filename_me((char *)tlist->data,&filepath);
            pallist = GetSelPal();
            while(pallist) {
                file = new FileInfo;
                file->fileid = cthrd.PrnQuote()++;
                /* file->packetn = 0;//没必要设置此字段 */
                file->fileattr = S_ISREG(st.st_mode) ? IPMSG_FILE_REGULAR :
                                                         IPMSG_FILE_DIR;
                file->filesize = filesize;
                file->filepath = g_strdup((char *)tlist->data);
                file->filectime = uint32_t(st.st_ctime);
                file->filenum = filenum;
                file->fileown = (PalInfo *)(pallist->data);
                /* 加入文件信息到中心节点 */
                pthread_mutex_lock(cthrd.GetMutex());
                cthrd.AttachFileToPrivate(file);
                pthread_mutex_unlock(cthrd.GetMutex());
                /* 添加数据 */
                gtk_list_store_append(GTK_LIST_STORE(model), &iter);
                gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, pixbuf,1, filename,
                                   2,numeric_to_size(filesize),3,tlist->data,
                                   4,file,5,file->fileown->name, -1);
                pallist = g_slist_next(pallist);
            }
            filenum++;
            /* 转到下一个文件节点 */
            tlist = g_slist_next(tlist);
        }
        //计算待发送文件总计大小
        totalsendsize = 0;
        gtk_tree_model_get_iter_first(model, &iter);
        do { //遍历待发送model
            gtk_tree_model_get(model, &iter,4,&file, -1);
            totalsendsize += file->filesize;
        } while (gtk_tree_model_iter_next(model, &iter));
        /* 释放文件图标 */
        if (rpixbuf)
                g_object_unref(rpixbuf);
        if (dpixbuf)
                g_object_unref(dpixbuf);

        pbar = GTK_WIDGET(g_datalist_get_data(&widset, "file-send-progress-bar-widget"));
        progresstip = g_strdup_printf("%s To Send.",numeric_to_size(totalsendsize));
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbar), _(progresstip));
        g_free(progresstip);
}

/*
 * 主窗口的信号连接
 */
void DialogBase::MainWindowSignalSetup(GtkWidget *window)
{
        g_object_set_data(G_OBJECT(window), "session-class", this);
        g_signal_connect_swapped(window, "destroy",
                                 G_CALLBACK(DialogDestory), this);
        g_signal_connect_swapped(window, "drag-data-received",
                                 G_CALLBACK(DragDataReceived), this);
        g_signal_connect(window, "configure-event",
                         G_CALLBACK(WindowConfigureEvent), &dtset);
        g_signal_connect(window, "focus-in-event",
                         G_CALLBACK(ClearNotify), NULL);
}

/**
 * 创建消息输入区域.
 * @return 主窗体
 */
GtkWidget *DialogBase::CreateInputArea()
{
        GtkWidget *frame, *box, *sw;
        GtkWidget *hbb, *button;
        GtkWidget *widget, *window;

        frame = gtk_frame_new(NULL);
        gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
        box = gtk_vbox_new(FALSE, 0);
        gtk_container_add(GTK_CONTAINER(frame), box);

        /* 接受输入 */
        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                                 GTK_SHADOW_ETCHED_IN);
        gtk_box_pack_start(GTK_BOX(box), sw, TRUE, TRUE, 0);
        widget = gtk_text_view_new();
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(widget), GTK_WRAP_WORD);
        gtk_drag_dest_add_uri_targets(widget);
        gtk_container_add(GTK_CONTAINER(sw), widget);
        g_signal_connect_swapped(widget, "drag-data-received",
                         G_CALLBACK(DragDataReceived), this);
        g_datalist_set_data(&widset, "input-textview-widget", widget);

        /* 功能按钮 */
        window = GTK_WIDGET(g_datalist_get_data(&widset, "window-widget"));
        hbb = gtk_hbutton_box_new();
        gtk_button_box_set_layout(GTK_BUTTON_BOX(hbb), GTK_BUTTONBOX_END);
        gtk_box_pack_start(GTK_BOX(box), hbb, FALSE, FALSE, 0);
        button = gtk_button_new_with_label(_("Close"));
        gtk_box_pack_end(GTK_BOX(hbb), button, FALSE, FALSE, 0);
        g_signal_connect_swapped(button, "clicked",
                         G_CALLBACK(gtk_widget_destroy), window);
        button = gtk_button_new_with_label(_("Send"));
        gtk_widget_add_accelerator(button, "clicked", accel, GDK_Return,
                 FLAG_ISSET(progdt.flags, 4) ? GdkModifierType(0) : GDK_CONTROL_MASK,
                 GTK_ACCEL_VISIBLE);
        gtk_box_pack_end(GTK_BOX(hbb), button, FALSE, FALSE, 0);
        g_signal_connect_swapped(button, "clicked", G_CALLBACK(SendMessage), this);

        return frame;
}

/**
 * 创建聊天历史记录区域.
 * @return 主窗体.
 */
GtkWidget *DialogBase::CreateHistoryArea()
{
        GtkWidget *frame, *sw;
        GtkWidget *widget;

        frame = gtk_frame_new(_("Chat History"));
        gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                                 GTK_SHADOW_ETCHED_IN);
        gtk_container_add(GTK_CONTAINER(frame), sw);

        widget = gtk_text_view_new_with_buffer(grpinf->buffer);
        gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(widget), FALSE);
        gtk_text_view_set_editable(GTK_TEXT_VIEW(widget), FALSE);
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(widget), GTK_WRAP_WORD);
        gtk_container_add(GTK_CONTAINER(sw), widget);
        g_signal_connect(widget, "key-press-event",
                 G_CALLBACK(textview_key_press_event), NULL);
        g_signal_connect(widget, "event-after",
                 G_CALLBACK(textview_event_after), NULL);
        g_signal_connect(widget, "motion-notify-event",
                 G_CALLBACK(textview_motion_notify_event), NULL);
        g_signal_connect(widget, "visibility-notify-event",
                 G_CALLBACK(textview_visibility_notify_event), NULL);
        g_datalist_set_data(&widset, "history-textview-widget", widget);

        /* 滚动消息到最末位置 */
        ScrollHistoryTextview();

        return frame;
}

/**
 * 创建文件菜单.
 * @return 菜单
 */
GtkWidget *DialogBase::CreateFileMenu()
{
        GtkWidget *menushell, *window,*treeview;
        GtkWidget *menu, *menuitem;

        window = GTK_WIDGET(g_datalist_get_data(&widset, "window-widget"));
        menushell = gtk_menu_item_new_with_mnemonic(_("_File"));
        menu = gtk_menu_new();
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(menushell), menu);

        menuitem = gtk_menu_item_new_with_label(_("Attach File"));
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
        g_signal_connect_swapped(menuitem, "activate", G_CALLBACK(AttachRegular), this);
        gtk_widget_add_accelerator(menuitem, "activate", accel,
                                   GDK_S, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

        menuitem = gtk_menu_item_new_with_label(_("Attach Folder"));
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
        g_signal_connect_swapped(menuitem, "activate", G_CALLBACK(AttachFolder), this);
        gtk_widget_add_accelerator(menuitem, "activate", accel,
                                   GDK_D, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

        treeview = GTK_WIDGET(g_datalist_get_data(&widset, "file-send-treeview-widget"));
        menuitem = gtk_menu_item_new_with_label(_("Remove Selected"));
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
        g_signal_connect_swapped(menuitem, "activate",
                                 G_CALLBACK(RemoveSelectedFromTree), treeview);
        gtk_widget_add_accelerator(menuitem, "activate", accel,
                                   GDK_R, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);


        menuitem = gtk_tearoff_menu_item_new();
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

        menuitem = gtk_menu_item_new_with_label(_("Close"));
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
        g_signal_connect_swapped(menuitem, "activate",
                         G_CALLBACK(gtk_widget_destroy), window);
        gtk_widget_add_accelerator(menuitem, "activate", accel,
                                   GDK_W, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

        g_datalist_set_data(&widset, "file-menu",menu);
        return menushell;
}

/**
 * 创建帮助菜单.
 * @return 菜单
 */
GtkWidget *DialogBase::CreateHelpMenu()
{
        GtkWidget *menushell;
        GtkWidget *menu, *menuitem;

        menushell = gtk_menu_item_new_with_mnemonic(_("_Help"));
        menu = gtk_menu_new();
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(menushell), menu);

        menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, accel);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
        g_signal_connect(menuitem, "activate", G_CALLBACK(HelpDialog::AboutEntry), NULL);

        return menushell;
}

/**
 * 选择附件.
 * @param fileattr 文件类型
 * @return 文件链表
 */
GSList *DialogBase::PickEnclosure(uint32_t fileattr)
{
        GtkWidget *dialog, *parent;
        GtkFileChooserAction action;
        const char *title;
        GSList *list;

        if (GET_MODE(fileattr) == IPMSG_FILE_REGULAR) {
                action = GTK_FILE_CHOOSER_ACTION_OPEN;
                title = _("Choose enclosure files");
        } else {
                action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
                title = _("Choose enclosure folders");
        }
        parent = GTK_WIDGET(g_datalist_get_data(&widset, "dialog-widget"));

        dialog = gtk_file_chooser_dialog_new(title, GTK_WINDOW(parent), action,
                                 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
        gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
        gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), FALSE);
        gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), g_get_home_dir());

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
 * 发送附件消息.
 * @return 是否发送数据
 */
bool DialogBase::SendEnclosureMsg()
{
        GtkWidget *treeview;
        GtkTreeModel *model;
        GtkTreeIter iter;
        GSList *list;
        gchar *filepath;
        FileInfo *file;

        treeview = GTK_WIDGET(g_datalist_get_data(&widset, "file-send-treeview-widget"));
        model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
        if (!gtk_tree_model_get_iter_first(model, &iter))
                return false;

        /* 获取文件并发送 */
        list = NULL;
        do {
                gtk_tree_model_get(model, &iter, 3, &filepath,4,&file,-1);
                list = g_slist_append(list,file);
        } while (gtk_tree_model_iter_next(model, &iter));

        BroadcastEnclosureMsg(list);
        g_slist_free(list);
        timersend = gdk_threads_add_timeout(400, (GSourceFunc)UpdateFileSendUI, this);
        return true;
}


/**
 * 回馈消息.
 * @param msg 消息
 */
void DialogBase::FeedbackMsg(const gchar *msg)
{
        MsgPara para;
        ChipData *chip;

        /* 构建消息封装包 */
        para.pal = NULL;
        para.stype = MESSAGE_SOURCE_TYPE_SELF;
        para.btype = grpinf->type;
        chip = new ChipData;
        chip->type = MESSAGE_CONTENT_TYPE_STRING;
        chip->data = g_strdup(msg);
        para.dtlist = g_slist_append(NULL, chip);

        /* 交给某人处理吧 */
        cthrd.InsertMsgToGroupInfoItem(grpinf, &para);
}

/**
 * 添加常规文件附件.
 * @param dlgpr 对话框类
 */
void DialogBase::AttachRegular(DialogBase *dlgpr)
{
        GSList *list;

        if (!(list = dlgpr->PickEnclosure(IPMSG_FILE_REGULAR)))
                return;
        dlgpr->AttachEnclosure(list);
        g_slist_foreach(list, GFunc(g_free), NULL);
        g_slist_free(list);
}

/**
 * 添加目录文件附件.
 * @param dlgpr 对话框类
 */
void DialogBase::AttachFolder(DialogBase *dlgpr)
{
        GSList *list;

        if (!(list = dlgpr->PickEnclosure(IPMSG_FILE_DIR)))
                return;
        dlgpr->AttachEnclosure(list);
        g_slist_foreach(list, GFunc(g_free), NULL);
        g_slist_free(list);
}

/**
 * 清空聊天历史记录缓冲区.
 * @param dlgpr 对话框类
 */
void DialogBase::ClearHistoryBuffer(DialogBase *dlgpr)
{
        dlgpr->ClearHistoryTextView();
}

/**
 * 发送消息.
 * @param dlgpr 对话框类
 */
void DialogBase::SendMessage(DialogBase *dlgpr)
{
        dlgpr->SendEnclosureMsg();
        dlgpr->SendTextMsg();
        dlgpr->ScrollHistoryTextview();
}
/**
 * 对话框被摧毁的回调函数
 * @param dialog
 */
void DialogBase::DialogDestory(DialogBase *dialog)
{
        delete dialog;
}

/**
 * 清除提示,这个提示只是窗口闪动的提示
 */
gboolean DialogBase::ClearNotify(GtkWidget *window, GdkEventConfigure *event)
{
#if GTK_CHECK_VERSION(2,8,0)
    if (gtk_window_get_urgency_hint(GTK_WINDOW(window)))
        gtk_window_set_urgency_hint(GTK_WINDOW(window), FALSE);
#endif
    return FALSE;
}

/**
 * 拖拽事件响应函数.
 * @param dlgpr 对话框类
 * @param context the drag context
 * @param x where the drop happened
 * @param y where the drop happened
 * @param data the received data
 * @param info the info that has been registered with the target in the GtkTargetList
 * @param time the timestamp at which the data was received
 */
void DialogBase::DragDataReceived(DialogBase *dlgpr, GdkDragContext *context,
                                 gint x, gint y, GtkSelectionData *data,
                                 guint info, guint time)
{
        GtkWidget *widget;
        GSList *list;

        if (data->length <= 0 || data->format != 8) {
                gtk_drag_finish(context, FALSE, FALSE, time);
                return;
        }

        list = selection_data_get_path(data);   //获取所有文件
        dlgpr->AttachEnclosure(list);
        g_slist_foreach(list, GFunc(g_free), NULL);
        g_slist_free(list);
        widget = GTK_WIDGET(g_datalist_get_data(&dlgpr->widset,
                                 "enclosure-frame-widget"));
        gtk_widget_show(widget);

        gtk_drag_finish(context, TRUE, FALSE, time);
}


/**
 * 主对话框位置&大小改变的响应处理函数.
 * @param window 主窗口
 * @param event the GdkEventConfigure which triggered this signal
 * @param dtset data set
 * @return Gtk+库所需
 */
gboolean DialogBase::WindowConfigureEvent(GtkWidget *window,
                                 GdkEventConfigure *event, GData **dtset)
{
        g_datalist_set_data(dtset, "window-width", GINT_TO_POINTER(event->width));
        g_datalist_set_data(dtset, "window-height", GINT_TO_POINTER(event->height));

        return FALSE;
}

/**
 * 分割面板的分割位置改变的响应处理函数.
 * @param paned paned
 * @param pspec he GParamSpec of the property which changed
 * @param dtset data set
 */
void DialogBase::PanedDivideChanged(GtkWidget *paned, GParamSpec *pspec,
                                                         GData **dtset)
{
        const gchar *identify;
        gint position;

        identify = (const gchar *)g_object_get_data(G_OBJECT(paned), "position-name");
        position = gtk_paned_get_position(GTK_PANED(paned));
        g_datalist_set_data(dtset, identify, GINT_TO_POINTER(position));
}

/**
 *删除选定附件.
 * @param dlgpr 对话框类
 */
void DialogBase::RemoveSelectedFromTree(GtkWidget *widget)
{
    GList *list;
    GtkTreeSelection *TreeSel;
    GtkTreePath *path;
    GtkTreeModel *model;
    gchar *str_data;
    gboolean valid = 0;
    GtkTreeIter iter;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
    TreeSel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
    list = gtk_tree_selection_get_selected_rows(TreeSel,NULL);
    if(!list)
        return;
    while(list) {
        gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &iter,
                                (GtkTreePath *)g_list_nth(list, 0)->data);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, 2,"delete", -1);
        list = g_list_next(list);
    }
    g_list_free(list);
    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
    path = gtk_tree_model_get_path(GTK_TREE_MODEL(model), &iter);
    while(valid) {
        gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,2,&str_data,-1);
        if(!g_strcmp0(str_data,"delete"))
            gtk_list_store_remove(GTK_LIST_STORE(model),&iter);
        else
            gtk_tree_path_next(path);
        valid = gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &iter, path);
    }
}

/**
 *显示附件的TreeView的弹出菜单回调函数.
 * @param widget TreeView
 * @param event 事件
 */
gint DialogBase::EncosureTreePopup(GtkWidget *treeview,GdkEvent *event)
{
    GtkWidget *menu,*menuitem;
    GdkEventButton *event_button;

    menu = gtk_menu_new();
    menuitem = gtk_menu_item_new_with_label(_("Remove Selected"));
    g_signal_connect_swapped(menuitem, "activate",
                             G_CALLBACK(RemoveSelectedEnclosure), treeview);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

    if (event->type == GDK_BUTTON_PRESS) {
        event_button = (GdkEventButton *) event;
        if (event_button->button == 3) {
            gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                            event_button->button, event_button->time);
            gtk_widget_show(menuitem);
            return TRUE;
        }
    }
    return FALSE;
}
/**
 *从显示附件的TreeView删除选定行.
 * @param widget TreeView
 */
void DialogBase::RemoveSelectedEnclosure(GtkWidget *widget)
{
    GtkTreeModel *model;
    GtkTreeSelection *TreeSel;
    GtkTreeIter iter;
    FileInfo *file;
    DialogBase *dlg;
    GList *list;

    dlg = (DialogBase *)(g_object_get_data(G_OBJECT(widget),"dialog"));
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
    //从中心结点删除
    TreeSel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
    list = gtk_tree_selection_get_selected_rows(TreeSel,NULL);
    if(!list)
        return;
    while(list) {
        gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &iter,
                                (GtkTreePath *)g_list_nth(list, 0)->data);
        gtk_tree_model_get(model, &iter,4,&file, -1);
        dlg->totalsendsize -= file->filesize;
        cthrd.DelFileFromPrivate(file->fileid);
        list = g_list_next(list);
    }
    g_list_free(list);
    //从列表中删除
    RemoveSelectedFromTree(widget);
    //重新计算待发送文件大小
    dlg->UpdateFileSendUI(dlg);
}

/**
 * 创建文件发送区域.
 * @return 主窗体
 */
GtkWidget *DialogBase::CreateFileSendArea()
{
    GtkWidget *frame, *hbox, *vbox, *button ,*pbar, *sw, *treeview;
    GtkTreeModel *model;
    frame = gtk_frame_new(_("File to send."));
    g_datalist_set_data(&widset, "file-send-frame-widget", frame);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
    pbar = gtk_progress_bar_new();
    g_datalist_set_data(&widset, "file-send-progress-bar-widget", pbar);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbar),_("Sending progress."));
    hbox = gtk_hbox_new(FALSE,1);
    gtk_box_pack_start(GTK_BOX(hbox),pbar,TRUE,TRUE,0);
    button = gtk_button_new_with_label(_("Dirs"));
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,TRUE,0);
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(AttachFolder), this);
    button = gtk_button_new_with_label(_("Files"));
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,TRUE,0);
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(AttachRegular), this);
    button = gtk_button_new_with_label(_("Detail"));
    gtk_box_pack_end(GTK_BOX(hbox),button,FALSE,TRUE,0);
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(OpenTransDlg), NULL);
    vbox = gtk_vbox_new(FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
             GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                             GTK_SHADOW_ETCHED_IN);
    model = CreateFileSendModel();
    treeview = CreateFileSendTree(model);
    g_datalist_set_data_full(&mdlset, "enclosure-model", model,
                             GDestroyNotify(g_object_unref));
    g_datalist_set_data(&widset, "file-send-treeview-widget", treeview);
    //保存this指针，在后面消息响应函数中用到
    g_object_set_data(G_OBJECT(treeview), "dialog", this);
    gtk_container_add(GTK_CONTAINER(sw), treeview);
    gtk_box_pack_end(GTK_BOX(vbox),sw,TRUE,TRUE,0);
    gtk_container_add(GTK_CONTAINER(frame), vbox);

    return frame;
}
/**
 * 创建待发送文件树(FileSend-tree).
 * @param model FileSend-model
 * @return 待发送文件树
 */
GtkWidget *DialogBase::CreateFileSendTree(GtkTreeModel *model)
{
    GtkWidget *view;
    GtkTreeSelection *selection;
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;

    view = gtk_tree_view_new_with_model(model);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), TRUE);
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

    cell = gtk_cell_renderer_pixbuf_new();
    column = gtk_tree_view_column_new_with_attributes("",cell,"pixbuf",0,NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

    if(grpinf->type != GROUP_BELONG_TYPE_REGULAR) {
        cell = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(_("PeelName"),cell,"text",5,NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    }

    cell = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Name"),cell,"text",1,NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

    cell = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Size"),cell,"text",2,NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

    cell = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Path"),cell,"text",3,NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

    g_signal_connect_swapped(GTK_OBJECT(view), "button_press_event",
                    G_CALLBACK(EncosureTreePopup), view);
    return view;
}

/**
 * 创建待发送文件树底层数据结构.
 * @return FileSend-model
 * 0:图标 1:文件名 2:大小(string) 3:全文件名 4:文件信息(指针) 5:接收者
 * 没有专门加删除标记，用第2列作删除标记，(某行反正要删除，改就改了)
 */
GtkTreeModel *DialogBase::CreateFileSendModel()
{
    GtkListStore *model;

    model = gtk_list_store_new(6, GDK_TYPE_PIXBUF,G_TYPE_STRING,
                     G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER,G_TYPE_STRING);

    return GTK_TREE_MODEL(model);
}
/**
 * 更新本窗口文件发送UI.
 * @param treeview FileSend-treeview
 * @return FileSend-model
 * 让传输聊天窗口从传输状态窗口去取数据，而没有让文件数据发送类把数据传送到聊天窗口，
 * 这是因为考虑数据要发到本窗口，会存在窗口未打开或群聊状态等不确定因素,处理过程太复杂
 */
gboolean DialogBase::UpdateFileSendUI(DialogBase *dlggrp)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    char progresstip[MAX_BUFLEN];
    GtkTreeView *treeview;
    GtkWidget *pbar;
    float progress;
    int64_t sentsize;
    GtkIconTheme *theme;
    GdkPixbuf *pixbuf;
    const char *statusfile;
    FileInfo *file;

    theme = gtk_icon_theme_get_default();
    statusfile = "tip-finish";
    pixbuf = gtk_icon_theme_load_icon(theme, statusfile, MAX_ICONSIZE,
                                             GtkIconLookupFlags(0), NULL);
    treeview = GTK_TREE_VIEW(g_datalist_get_data(&(dlggrp->widset),
                                                 "file-send-treeview-widget"));
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    sentsize =0;
    if(gtk_tree_model_get_iter_first(model, &iter)) {
        do { //遍历待发送model
            gtk_tree_model_get(model, &iter,4,&file, -1);
            if(pixbuf && (file->finishedsize == file->filesize))
                 gtk_list_store_set(GTK_LIST_STORE(model), &iter,0,pixbuf,-1);
            sentsize += file->finishedsize;
        } while (gtk_tree_model_iter_next(model, &iter));
    }
    /* 调整进度显示UI */
    gtk_tree_view_columns_autosize(GTK_TREE_VIEW(treeview));
    pbar = GTK_WIDGET(g_datalist_get_data(&(dlggrp->widset),
                                          "file-send-progress-bar-widget"));

    if(dlggrp->totalsendsize == 0) {
        progress = 0;
        snprintf(progresstip, MAX_BUFLEN,_("Sending Progress."));
    } else {
        progress = percent(sentsize,dlggrp->totalsendsize)/100;
        snprintf(progresstip, MAX_BUFLEN,_("%s Of %s Sent."),
                 numeric_to_size(sentsize),numeric_to_size(dlggrp->totalsendsize));
    }
    if(progress == 1){
            g_source_remove(dlggrp->timersend);
            gtk_list_store_clear(GTK_LIST_STORE(model));
            snprintf(progresstip, MAX_BUFLEN,_("Mission Completed!"));
    }
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbar),progress);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbar),_(progresstip));
    return TRUE;
}
/**
 * 打开文件传输窗口.
 * @param dlgpr 对话框类
 */
void DialogBase::OpenTransDlg(DialogBase *dlgpr)
{
    mwin.OpenTransWindow();
}
