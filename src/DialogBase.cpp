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

extern ProgramData progdt;
extern CoreThread cthrd;

DialogBase::DialogBase(GroupInfo *grp)
        :widset(NULL), mdlset(NULL),dtset(NULL), accel(NULL), grpinf(grp)
{
        InitSublayerGeneral();
}

DialogBase::~DialogBase()
{
        ClearSublayerGeneral();
}

/**
 * 初始化底层数据.
 */
void DialogBase::InitSublayerGeneral()
{
        GtkTreeModel *model;

        g_datalist_init(&widset);
        g_datalist_init(&mdlset);
        g_datalist_init(&dtset);
        accel = gtk_accel_group_new();

        model = CreateEnclosureModel();
        g_datalist_set_data_full(&mdlset, "enclosure-model", model,
                                 GDestroyNotify(g_object_unref));

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
 * 显示附件.
 */
void DialogBase::ShowEnclosure()
{
        GtkWidget *widget;

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "enclosure-frame-widget"));
        gtk_widget_show(widget);
}

/**
 * 添加附件.
 * @param list 文件链表
 */
void DialogBase::AttachEnclosure(const GSList *list)
{
        GtkWidget *widget;
        GtkTreeModel *model;
        GtkTreeIter iter;
        GdkPixbuf *pixbuf, *rpixbuf, *dpixbuf;
        struct stat64 st;
        const GSList *tlist;

        /* 获取文件图标 */
        rpixbuf = obtain_pixbuf_from_stock(GTK_STOCK_FILE);
        dpixbuf = obtain_pixbuf_from_stock(GTK_STOCK_DIRECTORY);

        /* 插入附件树 */
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "enclosure-treeview-widget"));
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
                /* 添加数据 */
                gtk_list_store_append(GTK_LIST_STORE(model), &iter);
                gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, pixbuf,
                                                         1, tlist->data, -1);
                /* 转到下一个节点 */
                tlist = g_slist_next(tlist);
        }

        /* 释放文件图标 */
        if (rpixbuf)
                g_object_unref(rpixbuf);
        if (dpixbuf)
                g_object_unref(dpixbuf);
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
 * 创建附件区域.
 * @return 主窗体
 */
GtkWidget *DialogBase::CreateEnclosureArea()
{
        GtkWidget *frame, *sw;
        GtkWidget *widget;
        GtkTreeModel *model;

        frame = gtk_frame_new(_("Enclosure"));
        gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
        g_datalist_set_data(&widset, "enclosure-frame-widget", frame);
        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                         GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                                 GTK_SHADOW_ETCHED_IN);
        gtk_container_add(GTK_CONTAINER(frame), sw);

        model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "enclosure-model"));
        widget = CreateEnclosureTree(model);
        gtk_container_add(GTK_CONTAINER(sw), widget);
        g_datalist_set_data(&widset, "enclosure-treeview-widget", widget);

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
        GtkWidget *menushell, *window;
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

        menuitem = gtk_tearoff_menu_item_new();
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

        menuitem = gtk_menu_item_new_with_label(_("Close"));
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
        g_signal_connect_swapped(menuitem, "activate",
                         G_CALLBACK(gtk_widget_destroy), window);
        gtk_widget_add_accelerator(menuitem, "activate", accel,
                                   GDK_W, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

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
 * 创建附件树(enclosure-tree).
 * @param model enclosure-model
 * @return 附件树
 */
GtkWidget *DialogBase::CreateEnclosureTree(GtkTreeModel *model)
{
        GtkWidget *view;
        GtkTreeSelection *selection;
        GtkCellRenderer *cell;
        GtkTreeViewColumn *column;

        view = gtk_tree_view_new_with_model(model);
        gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);
        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
        gtk_tree_selection_set_mode(selection, GTK_SELECTION_NONE);

        column = gtk_tree_view_column_new();
        gtk_tree_view_column_set_resizable(column, TRUE);
        cell = gtk_cell_renderer_pixbuf_new();
        gtk_tree_view_column_pack_start(column, cell, FALSE);
        gtk_tree_view_column_set_attributes(column, cell, "pixbuf", 0, NULL);
        cell = gtk_cell_renderer_text_new();
        gtk_tree_view_column_pack_start(column, cell, TRUE);
        gtk_tree_view_column_set_attributes(column, cell, "text", 1, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

        return view;
}

/**
 * 附件树(enclosure-tree)底层数据结构.
 * 2,0 logo,1 path \n
 * 文件图标;文件路径
 * @return enclosure-model
 */
GtkTreeModel *DialogBase::CreateEnclosureModel()
{
        GtkListStore *model;

        model = gtk_list_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_STRING);

        return GTK_TREE_MODEL(model);
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
        GtkWidget *frame, *treeview;
        GtkTreeModel *model;
        GtkTreeIter iter;
        GSList *list;
        gchar *filepath;

        /* 考察附件区是否存在文件 */
        frame = GTK_WIDGET(g_datalist_get_data(&widset, "enclosure-frame-widget"));
        gtk_widget_hide(frame);
        treeview = GTK_WIDGET(g_datalist_get_data(&widset, "enclosure-treeview-widget"));
        model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
        if (!gtk_tree_model_get_iter_first(model, &iter))
                return false;

        /* 获取文件并发送 */
        list = NULL;
        do {
                gtk_tree_model_get(model, &iter, 1, &filepath, -1);
                list = g_slist_append(list, filepath);
        } while (gtk_tree_model_iter_next(model, &iter));
        gtk_list_store_clear(GTK_LIST_STORE(model));
        BroadcastEnclosureMsg(list);
        /* g_slist_foreach(list, GFunc(glist_delete_foreach), UNKNOWN); */
        g_slist_free(list);

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
        GtkWidget *widget;
        GSList *list;

        if (!(list = dlgpr->PickEnclosure(IPMSG_FILE_REGULAR)))
                return;
        dlgpr->AttachEnclosure(list);
        g_slist_foreach(list, GFunc(g_free), NULL);
        g_slist_free(list);
        widget = GTK_WIDGET(g_datalist_get_data(&dlgpr->widset,
                                         "enclosure-frame-widget"));
        gtk_widget_show(widget);
}

/**
 * 添加目录文件附件.
 * @param dlgpr 对话框类
 */
void DialogBase::AttachFolder(DialogBase *dlgpr)
{
        GtkWidget *widget;
        GSList *list;

        if (!(list = dlgpr->PickEnclosure(IPMSG_FILE_DIR)))
                return;
        dlgpr->AttachEnclosure(list);
        g_slist_foreach(list, GFunc(g_free), NULL);
        g_slist_free(list);
        widget = GTK_WIDGET(g_datalist_get_data(&dlgpr->widset,
                                         "enclosure-frame-widget"));
        gtk_widget_show(widget);
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
