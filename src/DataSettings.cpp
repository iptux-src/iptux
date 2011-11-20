//
// C++ Implementation: DataSettings
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "DataSettings.h"
#include "ProgramData.h"
#include "CoreThread.h"
#include "SoundSystem.h"
#include "callback.h"
#include "output.h"
#include "support.h"
#include "utils.h"
extern ProgramData progdt;
extern SoundSystem sndsys;

/**
 * 类构造函数.
 */
DataSettings::DataSettings():widset(NULL), mdlset(NULL)
{
        InitSublayer();
}

/**
 * 类析构函数.
 */
DataSettings::~DataSettings()
{
        ClearSublayer();
}

/**
 * 程序数据设置入口.
 * @param parent 父窗口指针
 */
void DataSettings::ResetDataEntry(GtkWidget *parent)
{
        DataSettings dset;
        GtkWidget *dialog;
        GtkWidget *note, *label;

        dialog = dset.CreateMainDialog(parent);

        /* 创建相关数据设置标签 */
        note = gtk_notebook_new();
        gtk_notebook_set_tab_pos(GTK_NOTEBOOK(note), GTK_POS_LEFT);
        gtk_notebook_set_scrollable(GTK_NOTEBOOK(note), TRUE);
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), note, TRUE, TRUE, 0);
        label = gtk_label_new(_("Personal"));
        gtk_notebook_append_page(GTK_NOTEBOOK(note), dset.CreatePersonal(), label);
        label = gtk_label_new(_("System"));
        gtk_notebook_append_page(GTK_NOTEBOOK(note), dset.CreateSystem(), label);
#ifdef HAVE_GST
        label = gtk_label_new(_("Sound"));
        gtk_notebook_append_page(GTK_NOTEBOOK(note), dset.CreateSound(), label);
#endif
        label = gtk_label_new(_("Network"));
        gtk_notebook_append_page(GTK_NOTEBOOK(note), dset.CreateNetwork(), label);

        /* 设置相关数据默认值 */
        dset.SetPersonalValue();
        dset.SetSystemValue();
#ifdef HAVE_GST
        dset.SetSoundValue();
#endif
        dset.SetNetworkValue();

        /* 运行对话框 */
        gtk_widget_show_all(dialog);
mark:   switch (gtk_dialog_run(GTK_DIALOG(dialog))) {
        case GTK_RESPONSE_OK:
                dset.ObtainPersonalValue();
                dset.ObtainSystemValue();
#ifdef HAVE_GST
                dset.ObtainSoundValue();
#endif
                dset.ObtainNetworkValue();
                progdt.WriteProgData();
                CoreThread::UpdateMyInfo();
                break;
        case GTK_RESPONSE_APPLY:
                dset.ObtainPersonalValue();
                dset.ObtainSystemValue();
#ifdef HAVE_GST
                dset.ObtainSoundValue();
#endif
                dset.ObtainNetworkValue();
                progdt.WriteProgData();
                CoreThread::UpdateMyInfo();
                goto mark;
        default:
                break;
        }
        gtk_widget_destroy(dialog);
}

/**
 * 初始化底层数据.
 */
void DataSettings::InitSublayer()
{
        GtkTreeModel *model;

        g_datalist_init(&widset);
        g_datalist_init(&mdlset);

        model = CreateIconModel();
        g_datalist_set_data_full(&mdlset, "icon-model", model,
                                 GDestroyNotify(g_object_unref));
        FillIconModel(model);
#ifdef HAVE_GST
        model = CreateSndModel();
        g_datalist_set_data_full(&mdlset, "sound-model", model,
                                 GDestroyNotify(g_object_unref));
        FillSndModel(model);
#endif
        model = CreateNetworkModel();
        g_datalist_set_data_full(&mdlset, "network-model", model,
                                  GDestroyNotify(g_object_unref));
        FillNetworkModel(model);
}

/**
 * 清空底层数据.
 */
void DataSettings::ClearSublayer()
{
        g_datalist_clear(&widset);
        g_datalist_clear(&mdlset);
}

/**
 * 创建主对话框.
 * @param parent 父窗口指针
 * @return 对话框
 */
GtkWidget *DataSettings::CreateMainDialog(GtkWidget *parent)
{
        GtkWidget *dialog;

        dialog = gtk_dialog_new_with_buttons(_("Preferences"),
                         GTK_WINDOW(parent),
                         GtkDialogFlags(GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR),
                         GTK_STOCK_OK, GTK_RESPONSE_OK,
                         GTK_STOCK_APPLY, GTK_RESPONSE_APPLY,
                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
        gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
        gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
        gtk_container_set_border_width(GTK_CONTAINER(dialog), 5);
        gtk_widget_set_size_request(dialog, 520, -1);
        g_datalist_set_data(&widset, "dialog-widget", dialog);

        return dialog;
}

/**
 * 创建与个人相关的数据设置窗体.
 * @return 主窗体
 */
GtkWidget *DataSettings::CreatePersonal()
{
        GtkWidget *box, *hbox;
        GtkWidget *frame, *sw;
        GtkWidget *label, *button, *widget;
        GtkTreeModel *model;

        box = gtk_vbox_new(FALSE, 0);
        /* 昵称 */
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
        label = gtk_label_new(_("Your nickname:"));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        widget = gtk_entry_new();
        g_object_set(widget, "has-tooltip", TRUE, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
        g_signal_connect(widget, "query-tooltip", G_CALLBACK(entry_query_tooltip),
                                                 _("Please input your nickname!"));
        g_datalist_set_data(&widset, "nickname-entry-widget", widget);
        /* 群组 */
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
        label = gtk_label_new(_("Your group name:"));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        widget = gtk_entry_new();
        g_object_set(widget, "has-tooltip", TRUE, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
        g_signal_connect(widget, "query-tooltip", G_CALLBACK(entry_query_tooltip),
                                         _("Please input your group name!"));
        g_datalist_set_data(&widset, "mygroup-entry-widget", widget);
        /* 头像 */
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
        label = gtk_label_new(_("Your face picture:"));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "icon-model"));
        widget = CreateIconTree(model);
        gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
        g_datalist_set_data(&widset, "myicon-combo-widget", widget);
        button = gtk_button_new_with_label("...");
        g_object_set_data(G_OBJECT(button), "icon-combo-widget", widget);
        gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
        g_signal_connect(button, "clicked", G_CALLBACK(AddNewIcon), &widset);
        /* 文件存档 */
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
        label = gtk_label_new(_("Save files to: "));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        widget = CreateArchiveChooser();
        gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
        g_datalist_set_data(&widset, "archive-chooser-widget", widget);

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 5);
        /* 个人形象照片 */
        NO_OPERATION_C
        frame = gtk_frame_new(_("Photo"));
        gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
        gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
        button = gtk_button_new();
        gtk_widget_set_size_request(button, MAX_PREVIEWSIZE, MAX_PREVIEWSIZE);
        gtk_container_add(GTK_CONTAINER(frame), button);
        g_signal_connect_swapped(button, "clicked", G_CALLBACK(ChoosePhoto), &widset);
        widget = gtk_image_new();
        gtk_container_add(GTK_CONTAINER(button), widget);
        g_datalist_set_data(&widset, "photo-image-widget", widget);
        /* 个性签名 */
        NO_OPERATION_C
        frame = gtk_frame_new(_("Signature"));
        gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
        gtk_box_pack_end(GTK_BOX(hbox), frame, TRUE, TRUE, 0);
        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                         GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                                 GTK_SHADOW_ETCHED_IN);
        gtk_container_add(GTK_CONTAINER(frame), sw);
        widget = gtk_text_view_new();
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(widget), GTK_WRAP_WORD_CHAR);
        gtk_container_add(GTK_CONTAINER(sw), widget);
        g_datalist_set_data(&widset, "sign-textview-widget", widget);

        return box;
}

/**
 * 创建与系统相关的数据设置窗体.
 * @return 主窗体
 */
GtkWidget *DataSettings::CreateSystem()
{
        GtkWidget *box, *hbox;
        GtkWidget *label, *button, *widget;
        GtkTreeModel *model;

        box = gtk_vbox_new(FALSE, 0);
        /* 候选编码 */
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
        label = gtk_label_new(_("Candidate network coding:"));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        widget = gtk_entry_new();
        g_object_set(widget, "has-tooltip", TRUE, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
        g_signal_connect(widget, "query-tooltip", G_CALLBACK(entry_query_tooltip),
                                                 _("Candidate network coding"));
        g_datalist_set_data(&widset, "codeset-entry-widget", widget);
        /* 首选编码 */
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
        label = gtk_label_new(_("Preference network coding:"));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        widget = gtk_entry_new();
        g_object_set(widget, "has-tooltip", TRUE, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
        g_signal_connect(widget, "query-tooltip", G_CALLBACK(entry_query_tooltip),
                         _("Preference network coding (You should be aware of "
                           "what you are doing if you want to modify it.)"));
        g_datalist_set_data(&widset, "encode-entry-widget", widget);
        /* 好友头像 */
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
        label = gtk_label_new(_("Pal's default face picture:"));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "icon-model"));
        widget = CreateIconTree(model);
        gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
        g_datalist_set_data(&widset, "palicon-combo-widget", widget);
        button = gtk_button_new_with_label("...");
        g_object_set_data(G_OBJECT(button), "icon-combo-widget", widget);
        gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
        g_signal_connect(button, "clicked", G_CALLBACK(AddNewIcon), &widset);
        /* 面板字体 */
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
        label = gtk_label_new(_("Panel font:"));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        widget = CreateFontChooser();
        gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
        g_datalist_set_data(&widset, "font-chooser-widget", widget);

        /* 有消息时直接弹出聊天窗口 */
        widget = gtk_check_button_new_with_label(
                    _("Automatically open the chat dialog"));
        gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
        g_datalist_set_data(&widset, "chat-check-widget", widget);
        /* 隐藏面板，只显示状态图标 */
        widget = gtk_check_button_new_with_label(
                         _("Automatically hide the panel after login"));
        gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
        g_datalist_set_data(&widset, "statusicon-check-widget", widget);
        /* 打开文件传输管理器 */
        widget = gtk_check_button_new_with_label(
                         _("Automatically open the File Transmission Management"));
        gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
        g_datalist_set_data(&widset, "transmission-check-widget", widget);
        /* enter键发送消息 */
        widget = gtk_check_button_new_with_label(
                         _("Use the 'Enter' key to send message"));
        gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
        g_datalist_set_data(&widset, "enterkey-check-widget", widget);
        /* 清空聊天历史记录 */
        widget = gtk_check_button_new_with_label(
                         _("Automatically clean up the chat history"));
        gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
        g_datalist_set_data(&widset, "history-check-widget", widget);
        /* 记录日志 */
        widget = gtk_check_button_new_with_label(
                         _("Save the chat history"));
        gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
        g_datalist_set_data(&widset, "log-check-widget", widget);
        /* 黑名单 */
        widget = gtk_check_button_new_with_label(
                         _("Use the Blacklist (NOT recommended)"));
        gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
        g_datalist_set_data(&widset, "blacklist-check-widget", widget);
        /* 过滤共享文件请求 */
        widget = gtk_check_button_new_with_label(
                         _("Filter the request of sharing files"));
        gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
        g_datalist_set_data(&widset, "shared-check-widget", widget);

        return box;
}

/**
 * 创建与声音相关的数据设置窗体.
 * @return 主窗体
 */
GtkWidget *DataSettings::CreateSound()
{
        GtkWidget *box, *hbox, *vbox;
        GtkWidget *frame, *sw, *chkbutton;
        GtkWidget *label, *button, *widget;
        GtkTreeSelection *selection;
        GtkTreeModel *model;

        box = gtk_vbox_new(FALSE, 0);
        /* 声音支持 */
        NO_OPERATION_C
        chkbutton = gtk_check_button_new_with_label(_("Activate the sound support"));
        gtk_box_pack_start(GTK_BOX(box), chkbutton, FALSE, FALSE, 3);
        g_datalist_set_data(&widset, "sound-check-widget", chkbutton);
        /* 音量调整 */
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
        g_signal_connect(chkbutton, "toggled", G_CALLBACK(AdjustSensitive), hbox);
        label = gtk_label_new(_("Volume Control: "));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        widget = gtk_hscale_new_with_range(0.0, 1.0, 0.01);
        gtk_scale_set_draw_value(GTK_SCALE(widget), FALSE);
        gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
        g_signal_connect(widget, "value-changed", G_CALLBACK(AdjustVolume), NULL);
        g_datalist_set_data(&widset, "volume-hscale-widget", widget);
        /* 声音事件 */
        NO_OPERATION_C
        frame = gtk_frame_new(_("Sound Event"));
        gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
        gtk_box_pack_start(GTK_BOX(box), frame, TRUE, TRUE, 3);
        g_signal_connect(chkbutton, "toggled", G_CALLBACK(AdjustSensitive), frame);
        vbox = gtk_vbox_new(FALSE, 0);
        gtk_container_add(GTK_CONTAINER(frame), vbox);
        /*/* 声音事件树 */
        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                         GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                                 GTK_SHADOW_ETCHED_IN);
        gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 0);
        model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "sound-model"));
        widget = CreateSndTree(model);
        gtk_container_add(GTK_CONTAINER(sw), widget);
        g_signal_connect(widget, "button-press-event", G_CALLBACK(PopupPickMenu), NULL);
        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
        g_signal_connect(selection, "changed",
                         G_CALLBACK(SndtreeSelectItemChanged), &widset);
        g_datalist_set_data(&widset, "sound-treeview-widget", widget);
        /*/* 声音测试 */
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
        widget = CreateSndChooser();
        gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 5);
        g_signal_connect(widget, "file-set", G_CALLBACK(ChooserResetSndtree), &widset);
        g_datalist_set_data(&widset, "sound-chooser-widget", widget);
        button = gtk_button_new_with_label(_("Test"));
        gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
        g_signal_connect_swapped(button, "clicked", G_CALLBACK(PlayTesting), &widset);
        button = gtk_button_new_with_label(_("Stop"));
        gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
        g_signal_connect(button, "clicked", G_CALLBACK(StopTesting), NULL);

        return box;
}

/**
 * 创建与网络相关的数据设置窗体.
 * @return 主窗体
 */
GtkWidget *DataSettings::CreateNetwork()
{
        char buf[MAX_BUFLEN];
        GtkWidget *box, *hbox, *vbox;
        GtkWidget *frame, *sw;
        GtkWidget *label, *button, *widget;
        GtkTreeModel *model;

        box = gtk_vbox_new(FALSE, 0);
        /* 接受输入 */
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
        label = gtk_label_new(_("From:"));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        widget = gtk_entry_new();
        g_object_set(widget, "has-tooltip", TRUE, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
        g_signal_connect(widget, "query-tooltip", G_CALLBACK(entry_query_tooltip),
                                         _("Beginning of the IP(v4) section"));
        g_signal_connect(widget, "insert-text", G_CALLBACK(entry_insert_numeric), NULL);
        g_datalist_set_data(&widset, "startip-entry-widget", widget);
        label = gtk_label_new(_("To:"));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        widget = gtk_entry_new();
        g_object_set(widget, "has-tooltip", TRUE, NULL);
        gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
        g_signal_connect(widget, "query-tooltip", G_CALLBACK(entry_query_tooltip),
                                                  _("End of the IP(v4) section"));
        g_signal_connect(widget, "insert-text", G_CALLBACK(entry_insert_numeric), NULL);
        g_datalist_set_data(&widset, "endip-entry-widget", widget);
        /* 增加&删除按钮 */
        hbox = gtk_hbutton_box_new();
        gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox), GTK_BUTTONBOX_SPREAD);
        gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
        snprintf(buf, MAX_BUFLEN, "%s↓↓", _("Add"));
        button = gtk_button_new_with_label(buf);
        gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE, 0);
        g_signal_connect_swapped(button, "clicked", G_CALLBACK(ClickAddIpseg), &widset);
        snprintf(buf, MAX_BUFLEN, "%s↑↑", _("Delete"));
        button = gtk_button_new_with_label(buf);
        gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
        g_signal_connect_swapped(button, "clicked", G_CALLBACK(ClickDelIpseg), &widset);
        /* 网段树&实用性按钮 */
        NO_OPERATION_C
        frame = gtk_frame_new(_("Added IP(v4) Section:"));
        gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
        gtk_box_pack_start(GTK_BOX(box), frame, TRUE, TRUE, 5);
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_container_add(GTK_CONTAINER(frame), hbox);
        /*/* 网段树 */
        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                         GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                                 GTK_SHADOW_ETCHED_IN);
        gtk_box_pack_start(GTK_BOX(hbox), sw, TRUE, TRUE, 0);
        model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "network-model"));
        widget = CreateNetworkTree(model);
        gtk_container_add(GTK_CONTAINER(sw), widget);
        g_datalist_set_data(&widset, "network-treeview-widget", widget);
        /*/* 实用性按钮 */
        vbox = gtk_vbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
        button = gtk_button_new_with_label(_("Import"));
        gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
        g_signal_connect_swapped(button, "clicked", G_CALLBACK(ImportNetSegment), this);
        button = gtk_button_new_with_label(_("Export"));
        gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
        g_signal_connect_swapped(button, "clicked", G_CALLBACK(ExportNetSegment), this);
        button = gtk_button_new_with_label(_("Clear"));
        gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
        g_signal_connect_swapped(button, "clicked", G_CALLBACK(ClearNetSegment), &mdlset);

        return box;
}

/**
 * 为界面设置与个人相关的数据
 */
void DataSettings::SetPersonalValue()
{
        char path[MAX_PATHLEN];
        GtkWidget *widget;
        GtkTreeModel *model;
        GtkTextBuffer *buffer;
        GdkPixbuf *pixbuf;
        gint active;

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "nickname-entry-widget"));
        gtk_entry_set_text(GTK_ENTRY(widget), progdt.nickname);
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "mygroup-entry-widget"));
        gtk_entry_set_text(GTK_ENTRY(widget), progdt.mygroup);
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "myicon-combo-widget"));
        model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
        active = IconfileGetItemPos(model, progdt.myicon);
        gtk_combo_box_set_active(GTK_COMBO_BOX(widget), active);
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "archive-chooser-widget"));
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(widget), progdt.path);
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "photo-image-widget"));
        snprintf(path, MAX_PATHLEN, "%s" PHOTO_PATH "/photo", g_get_user_config_dir());
        if ( (pixbuf = gdk_pixbuf_new_from_file_at_size(path, MAX_PREVIEWSIZE,
                                                 MAX_PREVIEWSIZE, NULL))) {
                gtk_image_set_from_pixbuf(GTK_IMAGE(widget), pixbuf);
                g_object_unref(pixbuf);
        }
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "sign-textview-widget"));
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
        gtk_text_buffer_set_text(buffer, progdt.sign, -1);
}

/**
 * 为界面设置与系统相关的数据
 */
void DataSettings::SetSystemValue()
{
        GtkWidget *widget;
        GtkTreeModel *model;
        gint active;

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "codeset-entry-widget"));
        gtk_entry_set_text(GTK_ENTRY(widget), progdt.codeset);
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "encode-entry-widget"));
        gtk_entry_set_text(GTK_ENTRY(widget), progdt.encode);
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "palicon-combo-widget"));
        model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
        active = IconfileGetItemPos(model, progdt.palicon);
        gtk_combo_box_set_active(GTK_COMBO_BOX(widget), active);
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "font-chooser-widget"));
        gtk_font_button_set_font_name(GTK_FONT_BUTTON(widget), progdt.font);
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "chat-check-widget"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
                                     FLAG_ISSET(progdt.flags, 7));
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "statusicon-check-widget"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
                                 FLAG_ISSET(progdt.flags, 6));
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "transmission-check-widget"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
                                 FLAG_ISSET(progdt.flags, 5));
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "enterkey-check-widget"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
                                 FLAG_ISSET(progdt.flags, 4));
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "history-check-widget"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
                                 FLAG_ISSET(progdt.flags, 3));
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "log-check-widget"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
                                 FLAG_ISSET(progdt.flags, 2));
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "blacklist-check-widget"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
                                 FLAG_ISSET(progdt.flags, 1));
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "shared-check-widget"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
                                 FLAG_ISSET(progdt.flags, 0));
}

/**
 * 为界面设置与声音相关的数据
 */
void DataSettings::SetSoundValue()
{
        GtkWidget *widget;
        GtkTreeSelection *selection;
        GtkTreeModel *model;
        GtkTreeIter iter;
        gchar *filepath;

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "sound-check-widget"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
                                 FLAG_ISSET(progdt.sndfgs, 0));
        gtk_toggle_button_toggled(GTK_TOGGLE_BUTTON(widget));
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "volume-hscale-widget"));
        gtk_range_set_value(GTK_RANGE(widget), progdt.volume);
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "sound-treeview-widget"));
        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
        if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
                widget = GTK_WIDGET(g_datalist_get_data(&widset, "sound-chooser-widget"));
                gtk_tree_model_get(model, &iter, 2, &filepath, -1);
                gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widget), filepath);
                g_free(filepath);
        }
}

/**
 * 为界面设置与网络相关的数据
 */
void DataSettings::SetNetworkValue()
{
}

/**
 * 头像树(icon-tree)底层数据结构.
 * 2,0 icon,1 iconfile \n
 * 头像;文件名(带后缀) \n
 * @return icon-model
 */
GtkTreeModel *DataSettings::CreateIconModel()
{
        GtkListStore *model;

        model = gtk_list_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_STRING);

        return GTK_TREE_MODEL(model);
}

/**
 * 声音树(sound-tree)底层数据结构.
 * 3,0 toggled,1 comment,2 path \n
 * 是否被选中;用途注释;文件路径 \n
 * @return sound-model
 */
GtkTreeModel *DataSettings::CreateSndModel()
{
        GtkListStore *model;

        model = gtk_list_store_new(3, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING);

        return GTK_TREE_MODEL(model);
}

/**
 * 网络树(network-tree)底层数据结构.
 * 3,0 startip,1 endip,2 description \n
 * 起始IP;终止IP;描述 \n
 * @return network-model
 */
GtkTreeModel *DataSettings::CreateNetworkModel()
{
        GtkListStore *model;

        model = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(model),
                         GtkTreeIterCompareFunc(NetworkTreeCompareFunc),
                         NULL, NULL);
        gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model),
                         GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
                         GTK_SORT_ASCENDING);

        return GTK_TREE_MODEL(model);
}

/**
 * 为头像树(icon-tree)填充底层数据.
 * @param model icon-model
 */
void DataSettings::FillIconModel(GtkTreeModel *model)
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
 * 为声音树(sound-tree)填充底层数据.
 * @param model sound-model
 */
void DataSettings::FillSndModel(GtkTreeModel *model)
{
        GtkTreeIter iter;

        gtk_list_store_append(GTK_LIST_STORE(model), &iter);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                         0, FLAG_ISSET(progdt.sndfgs, 2),
                         1, _("Transfer finished"),
                         2, progdt.transtip, -1);
        gtk_list_store_append(GTK_LIST_STORE(model), &iter);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                         0, FLAG_ISSET(progdt.sndfgs, 1),
                         1, _("Message received"),
                         2, progdt.msgtip, -1);
}

/**
 * 为网络树(network-tree)填充底层数据.
 * @param model network-model
 * @note 与修改此链表的代码段是串行关系，无需加锁
 */
void DataSettings::FillNetworkModel(GtkTreeModel *model)
{
        GtkTreeIter iter;
        GSList *tlist;
        NetSegment *pns;

        tlist = progdt.netseg;
        while (tlist) {
                pns = (NetSegment *)tlist->data;
                gtk_list_store_append(GTK_LIST_STORE(model), &iter);
                gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, pns->startip,
                                        1, pns->endip, 2, pns->description, -1);
                tlist = g_slist_next(tlist);
        }
}

/**
 * 创建头像树(icon-tree).
 * @param model icon-model
 * @return 头像树
 */
GtkWidget *DataSettings::CreateIconTree(GtkTreeModel *model)
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
 * 创建声音树(sound-tree).
 * @param model sound-model
 * @return 声音树
 */
GtkWidget *DataSettings::CreateSndTree(GtkTreeModel *model)
{
        GtkWidget *view;
        GtkCellRenderer *cell;
        GtkTreeViewColumn *column;
        GtkTreeSelection *selection;
        GtkTreePath *path;

        view = gtk_tree_view_new_with_model(model);
        gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), TRUE);

        cell = gtk_cell_renderer_toggle_new();
        column = gtk_tree_view_column_new_with_attributes(_("Play"), cell,
                                                         "active", 0, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
        g_signal_connect_swapped(cell, "toggled", G_CALLBACK(model_turn_select), model);

        cell = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(_("Event"), cell,
                                                         "text", 1, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

        /* 函数ChooserResetView()要求必须选择一项 */
        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
        gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);
        path = gtk_tree_path_new_from_string("0");
        gtk_tree_selection_select_path(selection, path);
        gtk_tree_path_free(path);

        return view;
}

/**
 * 创建网络树(network-tree).
 * @param model network-model
 * @return 网络树
 */
GtkWidget *DataSettings::CreateNetworkTree(GtkTreeModel *model)
{
        GtkWidget *view;
        GtkCellRenderer *cell;
        GtkTreeViewColumn *column;
        GtkTreeSelection *selection;

        view = gtk_tree_view_new_with_model(model);
        gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), TRUE);
        gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(view), TRUE);
        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
        gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

        cell = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(_("From"), cell,
                                                         "text", 0, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

        cell = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(_("To"), cell,
                                                         "text", 1, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

        cell = gtk_cell_renderer_text_new();
        g_object_set(cell, "editable", TRUE, NULL);
        g_object_set_data(G_OBJECT(cell), "column-number", GINT_TO_POINTER(2));
        column = gtk_tree_view_column_new_with_attributes(_("Description"), cell,
                                                         "text", 2, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
        g_signal_connect(cell, "edited", G_CALLBACK(CellEditText), model);

        return view;
}

/**
 * 创建文件归档选择器.
 * @return 选择器
 */
GtkWidget *DataSettings::CreateArchiveChooser()
{
        GtkWidget *chooser;

        chooser = gtk_file_chooser_button_new(_("Please select download folder"),
                                         GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
        gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(chooser), FALSE);
        gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(chooser), FALSE);
        gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(chooser), TRUE);

        return chooser;
}

/**
 * 创建字体选择器.
 * @return 选择器
 */
GtkWidget *DataSettings::CreateFontChooser()
{
        GtkWidget *chooser;

        chooser = gtk_font_button_new();
        gtk_font_button_set_show_style(GTK_FONT_BUTTON(chooser), TRUE);
        gtk_font_button_set_show_size(GTK_FONT_BUTTON(chooser), TRUE);
        gtk_font_button_set_use_font(GTK_FONT_BUTTON(chooser), TRUE);
        gtk_font_button_set_use_size(GTK_FONT_BUTTON(chooser), TRUE);
        gtk_font_button_set_title(GTK_FONT_BUTTON(chooser), _("Select Font"));

        return chooser;
}

/**
 * 创建声音文件选择器.
 * @return 选择器
 */
GtkWidget *DataSettings::CreateSndChooser()
{
        GtkWidget *chooser;

        chooser = gtk_file_chooser_button_new(_("Please select a sound file"),
                                                 GTK_FILE_CHOOSER_ACTION_OPEN);
        gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(chooser), TRUE);
        gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(chooser), FALSE);

        return chooser;
}

/**
 * 获取与个人相关的数据.
 */
void DataSettings::ObtainPersonalValue()
{
        GtkWidget *widget;
        GdkPixbuf *pixbuf;
        GtkTextBuffer *buffer;
        GtkTextIter start, end;
        GtkTreeModel *model;
        GtkTreeIter iter;
        char path[MAX_PATHLEN], *file;
        const gchar *text;
        gint active;

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "nickname-entry-widget"));
        if (*(text = gtk_entry_get_text(GTK_ENTRY(widget))) != '\0') {
                g_free(progdt.nickname);
                progdt.nickname = g_strdup(text);
        }

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "mygroup-entry-widget"));
        if (*(text = gtk_entry_get_text(GTK_ENTRY(widget))) != '\0') {
                g_free(progdt.mygroup);
                progdt.mygroup = g_strdup(text);
        } else
                *progdt.mygroup = '\0';

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "myicon-combo-widget"));
        model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
        active = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
        if (active != -1) {
            snprintf(path, MAX_PATHLEN, "%d", active);
            gtk_tree_model_get_iter_from_string(model, &iter, path);
            gtk_tree_model_get(model, &iter, 1, &file, -1);
            if (strcmp(progdt.myicon, file) != 0) {
                    snprintf(path, MAX_PATHLEN, __PIXMAPS_PATH "/icon/%s", file);
                    if (access(path, F_OK) != 0) {
                            g_free(file);
                            g_free(progdt.myicon);
                            progdt.myicon = g_strdup("my-icon");
                            snprintf(path, MAX_PATHLEN, "%s" ICON_PATH "/my-icon",
                                                     g_get_user_config_dir());
                            gtk_tree_model_get(model, &iter, 0, &pixbuf, -1);
                            gdk_pixbuf_save(pixbuf, path, "png", NULL, NULL);
                            gtk_icon_theme_add_builtin_icon(progdt.myicon,
                                                     MAX_ICONSIZE, pixbuf);
                            g_object_unref(pixbuf);
                    } else {
                            g_free(progdt.myicon);
                            progdt.myicon = file;
                    }
            } else
                    g_free(file);
        }

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "archive-chooser-widget"));
        g_free(progdt.path);
        progdt.path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "sign-textview-widget"));
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        g_free(progdt.sign);
        progdt.sign = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
}

/**
 * 获取与系统相关的数据.
 */
void DataSettings::ObtainSystemValue()
{
        GtkWidget *widget;
        GdkPixbuf *pixbuf;
        GtkTreeModel *model;
        GtkTreeIter iter;
        char path[MAX_PATHLEN], *file;
        gchar *text;
        gint active;

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "codeset-entry-widget"));
        text = gtk_editable_get_chars(GTK_EDITABLE(widget), 0, -1);
        g_strstrip(text);
        if (*text != '\0') {
                g_free(progdt.codeset);
                progdt.codeset = text;
        } else
                g_free(text);

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "encode-entry-widget"));
        text = gtk_editable_get_chars(GTK_EDITABLE(widget), 0, -1);
        g_strstrip(text);
        if (*text != '\0') {
                g_free(progdt.encode);
                progdt.encode =text;
        } else
                g_free(text);

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "palicon-combo-widget"));
        model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
        active = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
        if(active != -1) {
            snprintf(path, MAX_PATHLEN, "%d", active);
            gtk_tree_model_get_iter_from_string(model, &iter, path);
            gtk_tree_model_get(model, &iter, 1, &file, -1);
            if (strcmp(progdt.palicon, file) != 0) {
                    snprintf(path, MAX_PATHLEN, __PIXMAPS_PATH "/icon/%s", file);
                    if (access(path, F_OK) != 0) {
                            g_free(file);
                            g_free(progdt.palicon);
                            progdt.palicon = g_strdup("pal-icon");
                            snprintf(path, MAX_PATHLEN, "%s" ICON_PATH "/pal-icon",
                                                     g_get_user_config_dir());
                            gtk_tree_model_get(model, &iter, 0, &pixbuf, -1);
                            gdk_pixbuf_save(pixbuf, path, "png", NULL, NULL);
                            gtk_icon_theme_add_builtin_icon(progdt.palicon,
                                                     MAX_ICONSIZE, pixbuf);
                            g_object_unref(pixbuf);
                    } else {
                            g_free(progdt.palicon);
                            progdt.palicon = file;
                    }
            } else
                    g_free(file);
        }

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "font-chooser-widget"));
        g_free(progdt.font);
        progdt.font = g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(widget)));

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "chat-check-widget"));
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
            FLAG_SET(progdt.flags, 7);
        else
            FLAG_CLR(progdt.flags, 7);

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "statusicon-check-widget"));
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
                FLAG_SET(progdt.flags, 6);
        else
                FLAG_CLR(progdt.flags, 6);

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "transmission-check-widget"));
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
                FLAG_SET(progdt.flags, 5);
        else
                FLAG_CLR(progdt.flags, 5);

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "enterkey-check-widget"));
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
                FLAG_SET(progdt.flags, 4);
        else
                FLAG_CLR(progdt.flags, 4);

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "history-check-widget"));
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
                FLAG_SET(progdt.flags, 3);
        else
                FLAG_CLR(progdt.flags, 3);

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "log-check-widget"));
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
                FLAG_SET(progdt.flags, 2);
        else
                FLAG_CLR(progdt.flags, 2);

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "blacklist-check-widget"));
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
                FLAG_SET(progdt.flags, 1);
        else
                FLAG_CLR(progdt.flags, 1);

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "shared-check-widget"));
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
                FLAG_SET(progdt.flags, 0);
        else
                FLAG_CLR(progdt.flags, 0);
}

/**
 * 获取与声音相关的数据.
 */
void DataSettings::ObtainSoundValue()
{
        GtkWidget *widget;
        GtkTreeModel *model;
        GtkTreeIter iter;
        gboolean active;
        gchar *path;

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "sound-check-widget"));
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
                FLAG_SET(progdt.sndfgs, 0);
        else
                FLAG_CLR(progdt.sndfgs, 0);

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "volume-hscale-widget"));
        progdt.volume = gtk_range_get_value(GTK_RANGE(widget));

        /**
         * @see ::FillSndModel()，数据的获取应该与其保持一致.
         */
        widget = GTK_WIDGET(g_datalist_get_data(&widset, "sound-treeview-widget"));
        model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
        /*/* 获取文件传输完成的声音信息 */
        gtk_tree_model_get_iter_from_string(model, &iter, "0");
        gtk_tree_model_get(model, &iter, 0, &active, 2, &path, -1);
        if (active)
                FLAG_SET(progdt.sndfgs, 2);
        else
                FLAG_CLR(progdt.sndfgs, 2);
        g_free(progdt.transtip);
        progdt.transtip = path;
        /*/* 获取有消息到来的声音信息 */
        gtk_tree_model_get_iter_from_string(model, &iter, "1");
        gtk_tree_model_get(model, &iter, 0, &active, 2, &path, -1);
        if (active)
                FLAG_SET(progdt.sndfgs, 1);
        else
                FLAG_CLR(progdt.sndfgs, 1);
        g_free(progdt.msgtip);
        progdt.msgtip = path;
}

/**
 * 获取与网络相关的数据.
 */
void DataSettings::ObtainNetworkValue()
{
        GtkWidget *widget;
        GtkTreeModel *model;
        GtkTreeIter iter;
        NetSegment *ns;
        GSList *tlist;

        widget = GTK_WIDGET(g_datalist_get_data(&widset, "network-treeview-widget"));
        model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
        pthread_mutex_lock(&progdt.mutex);
        for (tlist = progdt.netseg; tlist; tlist = g_slist_next(tlist))
                delete (NetSegment *)tlist->data;
        g_slist_free(progdt.netseg);
        progdt.netseg = NULL;
        if (gtk_tree_model_get_iter_first(model, &iter)) {
                do {
                        ns = new NetSegment;
                        gtk_tree_model_get(model, &iter, 0, &ns->startip,
                                 1, &ns->endip, 2, &ns->description, -1);
                        progdt.netseg = g_slist_append(progdt.netseg, ns);
                } while (gtk_tree_model_iter_next(model, &iter));
        }
        pthread_mutex_unlock(&progdt.mutex);
}

/**
 * 写出网段数据到指定文件.
 * @param filename 文件名
 * @param list 网段数据链表
 */
void DataSettings::WriteNetSegment(const char *filename, GSList *list)
{
        GtkWidget *parent;
        GSList *tlist;
        NetSegment *pns;
        FILE *stream;

        if (!(stream = fopen(filename, "w"))) {
                parent = GTK_WIDGET(g_datalist_get_data(&widset, "dialog-widget"));
                pop_warning(parent, _("Fopen() file \"%s\" failed!\n%s"),
                                         filename, strerror(errno));
                return;
        }

        fprintf(stream, "#format (startIP - endIP //description)");
        tlist = list;
        while (tlist) {
                pns = (NetSegment *)tlist->data;
                fprintf(stream, "\n%s - %s //%s", pns->startip, pns->endip,
                                 pns->description ? pns->description : "");
                tlist = g_slist_next(tlist);
        }
        fclose(stream);
}

/**
 * 从指定文件读取网段数据.
 * @param filename 文件名
 * @retval list 网段数据链表指针，数据由此返回
 */
void DataSettings::ReadNetSegment(const char *filename, GSList **list)
{
        GtkWidget *parent;
        char buf[3][MAX_BUFLEN], *lineptr;
        in_addr_t ipv4;
        NetSegment *ns;
        FILE *stream;
        size_t n;

        if (!(stream = fopen(filename, "r"))) {
                parent = GTK_WIDGET(g_datalist_get_data(&widset, "dialog-widget"));
                pop_warning(parent, _("Fopen() file \"%s\" failed!\n%s"),
                                         filename, strerror(errno));
                return;
        }

        n = 0;
        lineptr = NULL;
        while (getline(&lineptr, &n, stream) != -1) {
                if (*(lineptr + strspn(lineptr, "\t\x20")) == '#')
                        continue;
                switch (sscanf(lineptr, "%s - %s //%s", buf[0], buf[1], buf[2])) {
                case 3:
                        if (inet_pton(AF_INET, buf[0], &ipv4) <= 0
                                 || inet_pton(AF_INET, buf[1], &ipv4) <= 0)
                                break;
                        ns = new NetSegment;
                        *list = g_slist_append(*list, ns);
                        ns->startip = g_strdup(buf[0]);
                        ns->endip = g_strdup(buf[1]);
                        ns->description = g_strdup(buf[2]);
                        break;
                case 2:
                        if (inet_pton(AF_INET, buf[0], &ipv4) <= 0
                                 || inet_pton(AF_INET, buf[1], &ipv4) <= 0)
                                break;
                        ns = new NetSegment;
                        *list = g_slist_append(*list, ns);
                        ns->startip = g_strdup(buf[0]);
                        ns->endip = g_strdup(buf[1]);
                        break;
                default:
                        break;
                }
        }
        g_free(lineptr);
        fclose(stream);
}

/**
 * 创建选择项的弹出菜单.
 * @param model model
 * @return 菜单
 */
GtkWidget *DataSettings::CreatePopupMenu(GtkTreeModel *model)
{
        GtkWidget *menu, *menuitem;

        menu = gtk_menu_new();

        menuitem = gtk_menu_item_new_with_label(_("Select All"));
        g_signal_connect_swapped(menuitem, "activate",
                         G_CALLBACK(model_select_all), model);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

        menuitem = gtk_menu_item_new_with_label(_("Reverse Select"));
        g_signal_connect_swapped(menuitem, "activate",
                         G_CALLBACK(model_turn_all), model);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

        menuitem = gtk_menu_item_new_with_label(_("Clear Up"));
        g_signal_connect_swapped(menuitem, "activate",
                         G_CALLBACK(model_clear_all), model);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

        return menu;
}

/**
 * 查询(pathname)文件在(model)中的位置，若没有则加入到后面.
 * @param model model
 * @param pathname 文件路径
 * @return 位置
 */
gint DataSettings::IconfileGetItemPos(GtkTreeModel *model, const char *pathname)
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
 * 弹出选择项的菜单.
 * @param treeview tree-view
 * @param event event
 * @return Gtk+库所需
 */
gboolean DataSettings::PopupPickMenu(GtkWidget *treeview, GdkEventButton *event)
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
 * 添加新的头像数据.
 * @param button button
 * @param widset widget set
 */
void DataSettings::AddNewIcon(GtkWidget *button, GData **widset)
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

/**
 * 选择个人形象照片.
 * @param widset widget set
 */
void DataSettings::ChoosePhoto(GData **widset)
{
        GtkWidget *image, *parent;
        GdkPixbuf *pixbuf;
        gchar path[MAX_PATHLEN];
        gchar *filename;

        parent = GTK_WIDGET(g_datalist_get_data(widset, "dialog-widget"));
        if (!(filename = choose_file_with_preview(
                 _("Please select a personal photo"), parent)))
                return;

        if ( (pixbuf = gdk_pixbuf_new_from_file(filename, NULL))) {
                snprintf(path, MAX_PATHLEN, "%s" PHOTO_PATH "/photo",
                                                 g_get_user_config_dir());
                pixbuf_shrink_scale_1(&pixbuf, MAX_PHOTOSIZE, MAX_PHOTOSIZE);
                gdk_pixbuf_save(pixbuf, path, "bmp", NULL, NULL);       //命中率极高，不妨直接保存
                image = GTK_WIDGET(g_datalist_get_data(widset, "photo-image-widget"));
                pixbuf_shrink_scale_1(&pixbuf, MAX_PREVIEWSIZE, MAX_PREVIEWSIZE);
                gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
                g_object_unref(pixbuf);
        }
        g_free(filename);
}

/**
 * 根据(chkbutton)的状态来设置(widget)的灵敏度.
 * @param chkbutton check-button
 * @param widget widget
 */
void DataSettings::AdjustSensitive(GtkWidget *chkbutton, GtkWidget *widget)
{
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkbutton)))
                gtk_widget_set_sensitive(widget, TRUE);
        else
                gtk_widget_set_sensitive(widget, FALSE);
}

/**
 * 调整声音系统的音量.
 * @param hscale hscale
 */
void DataSettings::AdjustVolume(GtkWidget *hscale)
{
        gdouble value;

        value = gtk_range_get_value(GTK_RANGE(hscale));
        sndsys.AdjustVolume(value);
}

/**
 * 声音树(sound-tree)选中项变更的响应函数.
 * @param selection tree-selection
 * @param widset widget set
 */
void DataSettings::SndtreeSelectItemChanged(GtkTreeSelection *selection, GData **widset)
{
        GtkWidget *widget;
        GtkTreeModel *model;
        GtkTreeIter iter;
        gchar *path;

        gtk_tree_selection_get_selected(selection, &model, &iter);
        gtk_tree_model_get(model, &iter, 2, &path, -1);
        widget = GTK_WIDGET(g_datalist_get_data(widset, "sound-chooser-widget"));
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widget), path);
        g_free(path);
}

/**
 * 重设声音文件的响应函数.
 * @param chooser file-chooser
 * @param widset widget set
 */
void DataSettings::ChooserResetSndtree(GtkWidget *chooser, GData **widset)
{
        GtkWidget *treeview;
        GtkTreeSelection *selection;
        GtkTreeModel *model;
        GtkTreeIter iter;
        gchar *path;

        treeview = GTK_WIDGET(g_datalist_get_data(widset, "sound-treeview-widget"));
        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
        path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
        gtk_tree_selection_get_selected(selection, &model, &iter);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, 2, path, -1);
        g_free(path);
}

/**
 * 播放测试.
 * @param widset widget set
 */
void DataSettings::PlayTesting(GData **widset)
{
        GtkWidget *widget;
        gchar *path;

        widget = GTK_WIDGET(g_datalist_get_data(widset, "sound-chooser-widget"));
        path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
        sndsys.Playing(path);
        g_free(path);
}

/**
 * 停止播放测试.
 */
void DataSettings::StopTesting()
{
        sndsys.Stop();
}

/**
 * 网络树(network-tree)排序比较函数.
 * @param model network-model
 * @param a A GtkTreeIter in model
 * @param b Another GtkTreeIter in model
 * @return 比较值
 */
gint DataSettings::NetworkTreeCompareFunc(GtkTreeModel *model,
                                 GtkTreeIter *a, GtkTreeIter *b)
{
        gchar *atext, *btext;
        gint result;

        gtk_tree_model_get(model, a, 0, &atext, -1);
        gtk_tree_model_get(model, b, 0, &btext, -1);
        result = strcmp(atext, btext);
        g_free(atext);
        g_free(btext);

        return result;
}

/**
 * 增加一个IP网段.
 * @param widset widget set
 */
void DataSettings::ClickAddIpseg(GData **widset)
{
        GtkWidget *startentry, *endentry, *treeview, *parent;
        GtkTreeModel *model;
        GtkTreeIter iter;
        const gchar *starttext, *endtext;
        in_addr_t startip, endip;

        /* 合法性检查 */
        parent = GTK_WIDGET(g_datalist_get_data(widset, "dialog-widget"));
        startentry = GTK_WIDGET(g_datalist_get_data(widset, "startip-entry-widget"));
        starttext = gtk_entry_get_text(GTK_ENTRY(startentry));
        if (inet_pton(AF_INET, starttext, &startip) <= 0) {
                gtk_widget_grab_focus(startentry);
                pop_warning(parent, _("\nIllegal IP(v4) address: %s!"), starttext);
                return;
        }
        endentry = GTK_WIDGET(g_datalist_get_data(widset, "endip-entry-widget"));
        endtext = gtk_entry_get_text(GTK_ENTRY(endentry));
        if (inet_pton(AF_INET, endtext, &endip) <= 0) {
                gtk_widget_grab_focus(endentry);
                pop_warning(parent, _("\nIllegal IP(v4) address: %s!"), endtext);
                return;
        }

        /* 加入网段树 */
        startip = ntohl(startip);
        endip = ntohl(endip);
        treeview = GTK_WIDGET(g_datalist_get_data(widset, "network-treeview-widget"));
        model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
        gtk_list_store_append(GTK_LIST_STORE(model), &iter);
        if (startip <= endip)
                gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                                 0, starttext, 1, endtext, -1);
        else
                gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                                 0, endtext, 1, starttext, -1);

        /* 扫尾 */
        gtk_widget_grab_focus(startentry);
        gtk_entry_set_text(GTK_ENTRY(startentry), "\0");
        gtk_entry_set_text(GTK_ENTRY(endentry), "\0");
}

/**
 * 删除一个IP网段.
 * @param widset widget set
 */
void DataSettings::ClickDelIpseg(GData **widset)
{
        GtkWidget *startentry, *endentry, *treeview;
        GtkTreeSelection *selection;
        GtkTreeModel *model;
        GtkTreeIter iter;
        gchar *starttext, *endtext;

        treeview = GTK_WIDGET(g_datalist_get_data(widset, "network-treeview-widget"));
        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
        model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
        if (!gtk_tree_model_get_iter_first(model, &iter))
                return;

        /* 删除所有被选中的项，并提取第一项数据 */
        starttext = endtext = NULL;
        do {
mark:           if (gtk_tree_selection_iter_is_selected(selection, &iter))
                {
                        if (!starttext)
                                gtk_tree_model_get(model, &iter, 0, &starttext,
                                                         1, &endtext, -1);
                        if (gtk_list_store_remove( GTK_LIST_STORE(model), &iter))
                                goto mark;
                        break;
                }
        } while (gtk_tree_model_iter_next(model, &iter));

        /* 把第一项数据填入输入框 */
        if (!starttext)
                return;
        startentry = GTK_WIDGET(g_datalist_get_data(widset, "startip-entry-widget"));
        gtk_entry_set_text(GTK_ENTRY(startentry), starttext);
        g_free(starttext);
        endentry = GTK_WIDGET(g_datalist_get_data(widset, "endip-entry-widget"));
        gtk_entry_set_text(GTK_ENTRY(endentry), endtext);
        g_free(endtext);
}

/**
 * 编辑cell.
 * @param renderer cell-renderer-text
 * @param path item-path
 * @param newtext new-text
 * @param model model
 */
void DataSettings::CellEditText(GtkCellRendererText *renderer, gchar *path,
                                 gchar *newtext, GtkTreeModel *model)
{
        GtkTreeIter iter;
        gint number;

        gtk_tree_model_get_iter_from_string(model, &iter, path);
        number = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(renderer), "column-number"));
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, number, newtext, -1);
}

/**
 * 导入网段数据.
 * @param dset 数据设置类
 */
void DataSettings::ImportNetSegment(DataSettings *dset)
{
        GtkWidget *dialog, *parent;
        GtkTreeModel *model;
        GtkTreeIter iter;
        gchar *filename;
        GSList *list, *tlist;
        NetSegment *pns;

        parent = GTK_WIDGET(g_datalist_get_data(&dset->widset, "dialog-widget"));
        dialog = gtk_file_chooser_dialog_new(_("Please select a file to import data"),
                         GTK_WINDOW(parent), GTK_FILE_CHOOSER_ACTION_OPEN,
                         GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
        gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), g_get_home_dir());

        switch (gtk_dialog_run(GTK_DIALOG(dialog))) {
        case GTK_RESPONSE_ACCEPT:
                model = GTK_TREE_MODEL(g_datalist_get_data(&dset->mdlset,
                                                         "network-model"));
                gtk_list_store_clear(GTK_LIST_STORE(model));
                list = NULL;
                filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
                dset->ReadNetSegment(filename, &list);
                g_free(filename);
                tlist = list;
                while (tlist) {
                        pns = (NetSegment *)tlist->data;
                        gtk_list_store_append(GTK_LIST_STORE(model), &iter);
                        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                                         0, pns->startip, 1, pns->endip,
                                         2, pns->description, -1);
                        tlist = g_slist_next(tlist);
                }
                for (tlist = list; tlist; tlist = g_slist_next(tlist))
                        delete (NetSegment *)tlist->data;
                g_slist_free(list);
        default:
                break;
        }
        gtk_widget_destroy(dialog);
}

/**
 * 导出网段数据.
 * @param dset 数据设置类
 */
void DataSettings::ExportNetSegment(DataSettings *dset)
{
        GtkWidget *dialog, *parent;
        GtkTreeModel *model;
        GtkTreeIter iter;
        gchar *filename;
        GSList *list, *tlist;
        NetSegment *ns;

        parent = GTK_WIDGET(g_datalist_get_data(&dset->widset, "dialog-widget"));
        dialog = gtk_file_chooser_dialog_new(_("Save data to file"),
                         GTK_WINDOW(parent), GTK_FILE_CHOOSER_ACTION_SAVE,
                         GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
        gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), g_get_home_dir());

        switch (gtk_dialog_run(GTK_DIALOG(dialog))) {
        case GTK_RESPONSE_ACCEPT:
                model = GTK_TREE_MODEL(g_datalist_get_data(&dset->mdlset,
                                                         "network-model"));
                if (!gtk_tree_model_get_iter_first(model, &iter))
                        break;
                list = NULL;
                do {
                        ns = new NetSegment;
                        gtk_tree_model_get(model, &iter, 0, &ns->startip, 1, &ns->endip,
                                                                 2, &ns->description, -1);
                        list = g_slist_append(list, ns);
                } while (gtk_tree_model_iter_next(model, &iter));
                filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
                dset->WriteNetSegment(filename, list);
                g_free(filename);
                for (tlist = list; tlist; tlist = g_slist_next(tlist))
                        delete (NetSegment *)tlist->data;
                g_slist_free(list);
        default:
                break;
        }
        gtk_widget_destroy(dialog);
}

/**
 * 清空网段数据.
 * @param mdlset model set
 */
void DataSettings::ClearNetSegment(GData **mdlset)
{
        GtkTreeModel *model;

        model = GTK_TREE_MODEL(g_datalist_get_data(mdlset, "network-model"));
        gtk_list_store_clear(GTK_LIST_STORE(model));
}
