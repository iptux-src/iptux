//
// C++ Implementation: DialogGroup
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "DialogGroup.h"
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
extern MainWindow mwin;
extern LogSystem lgsys;

/**
 * 类构造函数.
 * @param grp 群组信息
 */
DialogGroup::DialogGroup(GroupInfo *grp):widset(NULL), mdlset(NULL),
 dtset(NULL), accel(NULL), grpinf(grp)
{
	InitSublayer();
	ReadUILayout();
}

/**
 * 类析构函数.
 */
DialogGroup::~DialogGroup()
{
	WriteUILayout();
	ClearSublayer();
}

/**
 * 群组对话框入口.
 * @param grpinf 群组信息
 */
void DialogGroup::GroupDialogEntry(GroupInfo *grpinf)
{
	DialogGroup *dlggrp;
	GtkWidget *window, *widget;

	dlggrp = new DialogGroup(grpinf);
	window = dlggrp->CreateMainWindow();
	gtk_container_add(GTK_CONTAINER(window), dlggrp->CreateAllArea());
	gtk_widget_show_all(window);

	/* 将焦点置于文本输入框 */
	widget = GTK_WIDGET(g_datalist_get_data(&dlggrp->widset,
					 "input-textview-widget"));
	gtk_widget_grab_focus(widget);
	/* 隐藏附件窗体 */
	widget = GTK_WIDGET(g_datalist_get_data(&dlggrp->widset,
					 "enclosure-frame-widget"));
	gtk_widget_hide(widget);
	/* 从消息队列中移除 */
	pthread_mutex_lock(cthrd.GetMutex());
	if (cthrd.MsglineContainItem(grpinf)) {
		mwin.MakeItemBlinking(grpinf, FALSE);
		cthrd.PopItemFromMsgline(grpinf);
	}
	pthread_mutex_unlock(cthrd.GetMutex());

	/* delete dlggrp;//请不要这样做，此类将会在窗口被摧毁后自动释放 */
}

/**
 * 更新群组成员树(member-tree)指定项.
 * @param pal class PalInfo
 */
void DialogGroup::UpdatePalData(PalInfo *pal)
{
	GtkIconTheme *theme;
	GdkPixbuf *pixbuf;
	GtkWidget *widget;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gpointer data;
	gchar *file;

	/* 查询项所在的位置，若没有则添加 */
	widget = GTK_WIDGET(g_datalist_get_data(&widset, "member-treeview-widget"));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	data = NULL;	//防止可能出现的(data == pal)
	if (gtk_tree_model_get_iter_first(model, &iter)) {
		do {
			gtk_tree_model_get(model, &iter, 3, &data, -1);
			if (data == pal)
				break;
		} while (gtk_tree_model_iter_next(model, &iter));
	}
	if (data != pal) {
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, FALSE, 3, pal, -1);
	}

	/* 更新数据 */
	theme = gtk_icon_theme_get_default();
	file = iptux_erase_filename_suffix(pal->iconfile);
	pixbuf = gtk_icon_theme_load_icon(theme, file, MAX_ICONSIZE,
					 GtkIconLookupFlags(0), NULL);
	g_free(file);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 1, pixbuf, 2, pal->name, -1);
	if (pixbuf)
		g_object_unref(pixbuf);
}

/**
 * 插入项到群组成员树(member-tree).
 * @param pal class PalInfo
 */
void DialogGroup::InsertPalData(PalInfo *pal)
{
	GtkIconTheme *theme;
	GdkPixbuf *pixbuf;
	GtkWidget *widget;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *file;

	theme = gtk_icon_theme_get_default();
	file = iptux_erase_filename_suffix(pal->iconfile);
	pixbuf = gtk_icon_theme_load_icon(theme, file, MAX_ICONSIZE,
					 GtkIconLookupFlags(0), NULL);
	g_free(file);
	widget = GTK_WIDGET(g_datalist_get_data(&widset, "member-treeview-widget"));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, FALSE, 1, pixbuf,
						 2, pal->name, 3, pal, -1);
	if (pixbuf)
		g_object_unref(pixbuf);
}

/**
 * 从群组成员树(member-tree)删除指定项.
 * @param pal class PalInfo
 */
void DialogGroup::DelPalData(PalInfo *pal)
{
	GtkWidget *widget;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gpointer data;

	widget = GTK_WIDGET(g_datalist_get_data(&widset, "member-treeview-widget"));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	if (gtk_tree_model_get_iter_first(model, &iter)) {
		do {
			gtk_tree_model_get(model, &iter, 3, &data, -1);
			if (data == pal) {
				gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
				break;
			}
		} while (gtk_tree_model_iter_next(model, &iter));
	}
}

/**
 * 清除本群组所有好友数据.
 */
void DialogGroup::ClearAllPalData()
{
	GtkWidget *widget;
	GtkTreeModel *model;

	widget = GTK_WIDGET(g_datalist_get_data(&widset, "member-treeview-widget"));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	gtk_list_store_clear(GTK_LIST_STORE(model));
}

/**
 * 显示附件.
 */
void DialogGroup::ShowEnclosure()
{
	GtkWidget *widget;

	widget = GTK_WIDGET(g_datalist_get_data(&widset, "enclosure-frame-widget"));
	gtk_widget_show(widget);
}

/**
 * 添加附件.
 * @param list 文件链表
 */
void DialogGroup::AttachEnclosure(const GSList *list)
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

/**
 * 滚动聊天历史记录区.
 */
void DialogGroup::ScrollHistoryTextview()
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
 * 初始化底层数据.
 */
void DialogGroup::InitSublayer()
{
	GtkTreeModel *model;

	g_datalist_init(&widset);
	g_datalist_init(&mdlset);
	g_datalist_init(&dtset);
	accel = gtk_accel_group_new();

	model = CreateMemberModel();
	g_datalist_set_data_full(&mdlset, "member-model", model,
				 GDestroyNotify(g_object_unref));
	FillMemberModel(model);
	model = CreateEnclosureModel();
	g_datalist_set_data_full(&mdlset, "enclosure-model", model,
				 GDestroyNotify(g_object_unref));
}

/**
 * 清空底层数据.
 */
void DialogGroup::ClearSublayer()
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
 * 读取对话框的UI布局数据.
 */
void DialogGroup::ReadUILayout()
{
	GConfClient *client;
	gint numeric;

	client = gconf_client_get_default();

	numeric = gconf_client_get_int(client, GCONF_PATH "/group_window_width", NULL);
	numeric = numeric ? numeric : 550;
	g_datalist_set_data(&dtset, "window-width", GINT_TO_POINTER(numeric));
	numeric = gconf_client_get_int(client, GCONF_PATH "/group_window_height", NULL);
	numeric = numeric ? numeric : 500;
	g_datalist_set_data(&dtset, "window-height", GINT_TO_POINTER(numeric));

	numeric = gconf_client_get_int(client,
			 GCONF_PATH "/group_main_paned_divide", NULL);
	numeric = numeric ? numeric : 150;
	g_datalist_set_data(&dtset, "main-paned-divide", GINT_TO_POINTER(numeric));

	numeric = gconf_client_get_int(client,
			 GCONF_PATH "/group_historyinput_paned_divide", NULL);
	numeric = numeric ? numeric : 320;
	g_datalist_set_data(&dtset, "historyinput-paned-divide",
					 GINT_TO_POINTER(numeric));

	numeric = gconf_client_get_int(client,
			 GCONF_PATH "/group_memberenclosure_paned_divide", NULL);
	numeric = numeric ? numeric : 320;
	g_datalist_set_data(&dtset, "memberenclosure-paned-divide",
					 GINT_TO_POINTER(numeric));

	g_object_unref(client);
}

/**
 * 写出对话框的UI布局数据.
 */
void DialogGroup::WriteUILayout()
{
	GConfClient *client;
	gint numeric;

	client = gconf_client_get_default();

	numeric = GPOINTER_TO_INT(g_datalist_get_data(&dtset, "window-width"));
	gconf_client_set_int(client, GCONF_PATH "/group_window_width", numeric, NULL);
	numeric = GPOINTER_TO_INT(g_datalist_get_data(&dtset, "window-height"));
	gconf_client_set_int(client, GCONF_PATH "/group_window_height", numeric, NULL);

	numeric = GPOINTER_TO_INT(g_datalist_get_data(&dtset, "main-paned-divide"));
	gconf_client_set_int(client, GCONF_PATH "/group_main_paned_divide",
							 numeric, NULL);

	numeric = GPOINTER_TO_INT(g_datalist_get_data(&dtset,
				 "historyinput-paned-divide"));
	gconf_client_set_int(client, GCONF_PATH "/group_historyinput_paned_divide",
								 numeric, NULL);

	numeric = GPOINTER_TO_INT(g_datalist_get_data(&dtset,
				 "memberenclosure-paned-divide"));
	gconf_client_set_int(client, GCONF_PATH "/group_memberenclosure_paned_divide",
									 numeric, NULL);

	g_object_unref(client);
}

/**
 * 清空聊天历史记录.
 */
void DialogGroup::ClearHistoryTextView()
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
 * 创建主窗口.
 * @return 窗口
 */
GtkWidget *DialogGroup::CreateMainWindow()
{
	char buf[MAX_BUFLEN];
	GtkWidget *window;
	gint width, height;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	snprintf(buf, MAX_BUFLEN, _("Talk with the group %s"), grpinf->name);
	gtk_window_set_title(GTK_WINDOW(window), buf);
	width = GPOINTER_TO_INT(g_datalist_get_data(&dtset, "window-width"));
	height = GPOINTER_TO_INT(g_datalist_get_data(&dtset, "window-height"));
	gtk_window_set_default_size(GTK_WINDOW(window), width, height);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_add_accel_group(GTK_WINDOW(window), accel);
	g_datalist_set_data(&widset, "window-widget", window);

	grpinf->dialog = window;
	g_object_set_data(G_OBJECT(window), "session-class", this);
	g_signal_connect_swapped(window, "destroy", G_CALLBACK(DialogGroupDestroy), this);
	widget_enable_dnd_uri(window);
	g_signal_connect_swapped(window, "drag-data-received",
			 G_CALLBACK(DragDataReceived), this);
	g_signal_connect(window, "configure-event",
			 G_CALLBACK(WindowConfigureEvent), &dtset);

	return window;
}

/**
 * 创建所有区域.
 * @return 主窗体
 */
GtkWidget *DialogGroup::CreateAllArea()
{
	GtkWidget *box;
	GtkWidget *hpaned, *vpaned;
	gint position;

	box = gtk_vbox_new(FALSE, 0);

	/* 加入菜单条 */
	gtk_box_pack_start(GTK_BOX(box), CreateMenuBar(), FALSE, FALSE, 0);

	/* 加入主区域 */
	hpaned = gtk_hpaned_new();
	g_object_set_data(G_OBJECT(hpaned), "position-name",
				 (gpointer)"main-paned-divide");
	position = GPOINTER_TO_INT(g_datalist_get_data(&dtset, "main-paned-divide"));
	gtk_paned_set_position(GTK_PANED(hpaned), position);
	gtk_box_pack_start(GTK_BOX(box), hpaned, TRUE, TRUE, 0);
	g_signal_connect(hpaned, "notify::position",
			 G_CALLBACK(PanedDivideChanged), &dtset);
	/*/* 加入组成员&附件区域 */
	vpaned = gtk_vpaned_new();
	g_object_set_data(G_OBJECT(vpaned), "position-name",
		 (gpointer)"memberenclosure-paned-divide");
	position = GPOINTER_TO_INT(g_datalist_get_data(&dtset,
				 "memberenclosure-paned-divide"));
	gtk_paned_set_position(GTK_PANED(vpaned), position);
	gtk_paned_pack1(GTK_PANED(hpaned), vpaned, FALSE, TRUE);
	g_signal_connect(vpaned, "notify::position",
			 G_CALLBACK(PanedDivideChanged), &dtset);
	gtk_paned_pack1(GTK_PANED(vpaned), CreateMemberArea(), TRUE, TRUE);
	gtk_paned_pack2(GTK_PANED(vpaned), CreateEnclosureArea(), FALSE, TRUE);
	/*/* 加入聊天历史记录&输入区域 */
	vpaned = gtk_vpaned_new();
	g_object_set_data(G_OBJECT(vpaned), "position-name",
			 (gpointer)"historyinput-paned-divide");
	position = GPOINTER_TO_INT(g_datalist_get_data(&dtset,
				 "historyinput-paned-divide"));
	gtk_paned_set_position(GTK_PANED(vpaned), position);
	gtk_paned_pack2(GTK_PANED(hpaned), vpaned, TRUE, TRUE);
	g_signal_connect(vpaned, "notify::position",
			 G_CALLBACK(PanedDivideChanged), &dtset);
	gtk_paned_pack1(GTK_PANED(vpaned), CreateHistoryArea(), TRUE, TRUE);
	gtk_paned_pack2(GTK_PANED(vpaned), CreateInputArea(), FALSE, TRUE);

	return box;
}

/**
 * 创建菜单条.
 * @return 菜单条
 */
GtkWidget *DialogGroup::CreateMenuBar()
{
	GtkWidget *menubar;

	menubar = gtk_menu_bar_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), CreateFileMenu());
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), CreateToolMenu());
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), CreateHelpMenu());

	return menubar;
}

/**
 * 创建组成员区域.
 * @return 主窗体
 */
GtkWidget *DialogGroup::CreateMemberArea()
{
	GtkWidget *frame, *sw;
	GtkWidget *widget;
	GtkTreeModel *model;

	frame = gtk_frame_new(_("Member"));
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
	sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
			 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
						 GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(frame), sw);

	model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "member-model"));
	widget = CreateMemberTree(model);
	gtk_container_add(GTK_CONTAINER(sw), widget);
	g_signal_connect(widget, "button-press-event", G_CALLBACK(PopupPickMenu), NULL);
	g_signal_connect(widget, "row-activated",
			 G_CALLBACK(MembertreeItemActivated), NULL);
	g_datalist_set_data(&widset, "member-treeview-widget", widget);

	return frame;
}

/**
 * 创建附件区域.
 * @return 主窗体
 */
GtkWidget *DialogGroup::CreateEnclosureArea()
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
GtkWidget *DialogGroup::CreateHistoryArea()
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
 * 创建消息输入区域.
 * @return 主窗体
 */
GtkWidget *DialogGroup::CreateInputArea()
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
 * 群组成员树(member-tree)底层数据结构.
 * 4,0 toggled,1 icon,2 nickname,3 data \n
 * 是否被选中;好友头像;好友昵称;好友数据 \n
 * @return member-model
 */
GtkTreeModel *DialogGroup::CreateMemberModel()
{
	GtkListStore *model;

	model = gtk_list_store_new(4, G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF,
					 G_TYPE_STRING, G_TYPE_POINTER);
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(model),
		 GtkTreeIterCompareFunc(MemberTreeCompareByNameFunc),
		 NULL, NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model),
			 GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
			 GTK_SORT_ASCENDING);

	return GTK_TREE_MODEL(model);
}

/**
 * 附件树(enclosure-tree)底层数据结构.
 * 2,0 logo,1 path \n
 * 文件图标;文件路径
 * @return enclosure-model
 */
GtkTreeModel *DialogGroup::CreateEnclosureModel()
{
	GtkListStore *model;

	model = gtk_list_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_STRING);

	return GTK_TREE_MODEL(model);
}

/**
 * 为群组成员树(member-tree)填充底层数据.
 * @param model member-model
 */
void DialogGroup::FillMemberModel(GtkTreeModel *model)
{
	GtkIconTheme *theme;
	GdkPixbuf *pixbuf;
	GtkTreeIter iter;
	GSList *tlist;
	PalInfo *pal;
	char *file;

	theme = gtk_icon_theme_get_default();
	pthread_mutex_lock(cthrd.GetMutex());
	tlist = grpinf->member;
	while (tlist) {
		pal = (PalInfo *)tlist->data;
		file = iptux_erase_filename_suffix(pal->iconfile);
		pixbuf = gtk_icon_theme_load_icon(theme, file, MAX_ICONSIZE,
						 GtkIconLookupFlags(0), NULL);
		g_free(file);
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, TRUE,
				 1, pixbuf, 2, pal->name, 3, pal, -1);
		if (pixbuf)
			g_object_unref(pixbuf);
		tlist = g_slist_next(tlist);
	}
	pthread_mutex_unlock(cthrd.GetMutex());
}

/**
 * 创建群组成员树(member-tree).
 * @param model member-model
 * @return 群组树
 */
GtkWidget *DialogGroup::CreateMemberTree(GtkTreeModel *model)
{
	GtkWidget *view;
	GtkTreeSelection *selection;
	GtkCellRenderer *cell;
	GtkTreeViewColumn *column;

	view = gtk_tree_view_new_with_model(model);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), TRUE);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_NONE);

	cell = gtk_cell_renderer_toggle_new();
	g_signal_connect_swapped(cell, "toggled",
			 G_CALLBACK(model_turn_select), model);
	column = gtk_tree_view_column_new_with_attributes(_("Send"),
					 cell, "active", 0, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("Pals"));
	cell = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, cell, FALSE);
	gtk_tree_view_column_set_attributes(column, cell, "pixbuf", 1, NULL);
	cell = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, cell, FALSE);
	gtk_tree_view_column_set_attributes(column, cell, "text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	return view;
}

/**
 * 创建附件树(enclosure-tree).
 * @param model enclosure-model
 * @return 附件树
 */
GtkWidget *DialogGroup::CreateEnclosureTree(GtkTreeModel *model)
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
 * 创建文件菜单.
 * @return 菜单
 */
GtkWidget *DialogGroup::CreateFileMenu()
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

	menuitem = gtk_menu_item_new_with_label(_("Attach Folder"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect_swapped(menuitem, "activate", G_CALLBACK(AttachFolder), this);

	menuitem = gtk_tearoff_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_menu_item_new_with_label(_("Close"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect_swapped(menuitem, "activate",
			 G_CALLBACK(gtk_widget_destroy), window);

	return menushell;
}

/**
 * 创建工具菜单.
 * @return 菜单
 */
GtkWidget *DialogGroup::CreateToolMenu()
{
	GtkWidget *menushell;
	GtkWidget *menu, *submenu, *menuitem;
	GtkTreeModel *model;

	model = GTK_TREE_MODEL(g_datalist_get_data(&mdlset, "member-model"));
	menushell = gtk_menu_item_new_with_mnemonic(_("_Tools"));
	menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menushell), menu);

	/* 清空历史缓冲区 */
	NO_OPERATION_C
	menuitem = gtk_menu_item_new_with_label(_("Clear Buffer"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect_swapped(menuitem, "activate",
			 G_CALLBACK(ClearHistoryBuffer), this);

	/* 群组成员排序 */
	NO_OPERATION_C
	menuitem = gtk_menu_item_new_with_label(_("Sort"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	submenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), submenu);
	/*/* 按昵称排序 */
	NO_OPERATION_C
	menuitem = gtk_radio_menu_item_new_with_label(NULL, _("By Nickname"));
	g_object_set_data(G_OBJECT(menuitem), "compare-func",
			 (gpointer)MemberTreeCompareByNameFunc);
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
	g_signal_connect(menuitem, "toggled", G_CALLBACK(SetMemberTreeSortFunc), model);
	/*/* 按IP地址排序 */
	menuitem = gtk_radio_menu_item_new_with_label_from_widget(
			 GTK_RADIO_MENU_ITEM(menuitem), _("By IP"));
	g_object_set_data(G_OBJECT(menuitem), "compare-func",
			 (gpointer)MemberTreeCompareByIPFunc);
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
	g_signal_connect(menuitem, "toggled", G_CALLBACK(SetMemberTreeSortFunc), model);
	/*/* 分割符 */
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
	/*/* 升序 */
	NO_OPERATION_C
	menuitem = gtk_radio_menu_item_new_with_label(NULL, _("Ascending"));
	g_object_set_data(G_OBJECT(menuitem), "sort-type",
			 GINT_TO_POINTER(GTK_SORT_ASCENDING));
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
	g_signal_connect(menuitem, "toggled", G_CALLBACK(SetMemberTreeSortType), model);
	/*/* 降序 */
	menuitem = gtk_radio_menu_item_new_with_label_from_widget(
			 GTK_RADIO_MENU_ITEM(menuitem), _("Descending"));
	g_object_set_data(G_OBJECT(menuitem), "sort-type",
			 GINT_TO_POINTER(GTK_SORT_DESCENDING));
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
	g_signal_connect(menuitem, "toggled", G_CALLBACK(SetMemberTreeSortType), model);

	return menushell;
}

/**
 * 创建帮助菜单.
 * @return 菜单
 */
GtkWidget *DialogGroup::CreateHelpMenu()
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
GSList *DialogGroup::PickEnclosure(uint32_t fileattr)
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
bool DialogGroup::SendEnclosureMsg()
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
 * 发送文本消息.
 * @return 是否发送数据
 */
bool DialogGroup::SendTextMsg()
{
	GtkWidget *textview;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;
	gchar *msg;

	/* 考察缓冲区内是否存在数据 */
	textview = GTK_WIDGET(g_datalist_get_data(&widset, "input-textview-widget"));
	gtk_widget_grab_focus(textview);	//为下一次任务做准备
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	if (gtk_text_iter_equal(&start, &end))
		return false;

	/* 获取数据并发送 */
	msg = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
	gtk_text_buffer_delete(buffer, &start, &end);
	FeedbackMsg(msg);
	BroadcastTextMsg(msg);
	lgsys.CommunicateLog(NULL, "%s", msg);
	g_free(msg);

	return true;
}

/**
 * 回馈消息.
 * @param msg 消息
 */
void DialogGroup::FeedbackMsg(const gchar *msg)
{
	MsgPara para;
	ChipData *chip;

	/* 构建消息封装包 */
	para.pal = NULL;
	para.stype = ME_TYPE;
	para.btype = grpinf->type;
	chip = new ChipData;
	chip->type = STRING_TYPE;
	chip->data = g_strdup(msg);
	para.dtlist = g_slist_append(NULL, chip);

	/* 交给某人处理吧 */
	cthrd.InsertMsgToGroupInfoItem(grpinf, &para);
}

/**
 * 向选中的好友广播附件消息.
 * @param list 文件链表
 */
void DialogGroup::BroadcastEnclosureMsg(GSList *list)
{
	SendFile sfile;
	GtkWidget *widget;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean active;
	PalInfo *pal;
	GSList *plist;

	/* 考察是否有成员 */
	widget = GTK_WIDGET(g_datalist_get_data(&widset, "member-treeview-widget"));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	if (!gtk_tree_model_get_iter_first(model, &iter)) {
		/**
		 * @note 链表(list)的数据本来应该由(sfile.BcstFileInfoEntry())接手的，
		 * 既然已经没有那个机会了， 当然就只好在这儿手动释放了.
		 */
		g_slist_foreach(list, GFunc(g_free), NULL);
		return;
	}

	/* 向选中的成员发送附件 */
	plist = NULL;
	do {
		gtk_tree_model_get(model, &iter, 0, &active, 3, &pal, -1);
		if (active)
			plist = g_slist_append(plist, pal);
	} while (gtk_tree_model_iter_next(model, &iter));
	sfile.BcstFileInfoEntry(plist, list);
	g_slist_free(plist);
}

/**
 * 向选中的好友广播文本消息.
 * @param msg 文本消息
 */
void DialogGroup::BroadcastTextMsg(const gchar *msg)
{
	Command cmd;
	GtkWidget *widget;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean active;
	uint32_t opttype;
	PalInfo *pal;

	/* 考察是否有成员 */
	widget = GTK_WIDGET(g_datalist_get_data(&widset, "member-treeview-widget"));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	if (!gtk_tree_model_get_iter_first(model, &iter))
		return;

	/* 向选中的成员发送数据 */
	do {
		gtk_tree_model_get(model, &iter, 0, &active, 3, &pal, -1);
		if (active) {
			if (FLAG_ISSET(pal->flags, 0)) {
				switch (grpinf->type) {
				case BROADCAST_TYPE:
					opttype = IPTUX_BROADCASTOPT;
					break;
				case GROUP_TYPE:
					opttype = IPTUX_GROUPOPT;
					break;
				case SEGMENT_TYPE:
					opttype = IPTUX_SEGMENTOPT;
					break;
				case REGULAR_TYPE:
				default:
					opttype = IPTUX_REGULAROPT;
					break;
				}
				cmd.SendUnitMsg(cthrd.UdpSockQuote(), pal,
							 opttype, msg);
			} else
				cmd.SendGroupMsg(cthrd.UdpSockQuote(), pal, msg);
		}
	} while (gtk_tree_model_iter_next(model, &iter));
}

/**
 * 创建选择项的弹出菜单.
 * @param model model
 * @return 菜单
 */
GtkWidget *DialogGroup::CreatePopupMenu(GtkTreeModel *model)
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
 * 群组成员树(member-tree)按昵称排序的比较函数.
 * @param model member-model
 * @param a A GtkTreeIter in model
 * @param b Another GtkTreeIter in model
 * @return 比较值
 */
gint DialogGroup::MemberTreeCompareByNameFunc(GtkTreeModel *model,
					 GtkTreeIter *a, GtkTreeIter *b)
{
	PalInfo *apal, *bpal;
	gint result;

	gtk_tree_model_get(model, a, 3, &apal, -1);
	gtk_tree_model_get(model, b, 3, &bpal, -1);
	result = strcmp(apal->name, bpal->name);

	return result;
}

/**
 * 群组成员树(member-tree)按IP排序的比较函数.
 * @param model member-model
 * @param a A GtkTreeIter in model
 * @param b Another GtkTreeIter in model
 * @return 比较值
 */
gint DialogGroup::MemberTreeCompareByIPFunc(GtkTreeModel *model,
					 GtkTreeIter *a, GtkTreeIter *b)
{
	PalInfo *apal, *bpal;
	gint result;

	gtk_tree_model_get(model, a, 3, &apal, -1);
	gtk_tree_model_get(model, b, 3, &bpal, -1);
	result = ntohl(apal->ipv4) - ntohl(bpal->ipv4);

	return result;
}

/**
 * 设置群组成员树(member-tree)的比较函数.
 * @param menuitem radio-menu-item
 * @param model member-model
 */
void DialogGroup::SetMemberTreeSortFunc(GtkWidget *menuitem, GtkTreeModel *model)
{
	GtkTreeIterCompareFunc func;

	if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
		return;
	func = (GtkTreeIterCompareFunc)(g_object_get_data(G_OBJECT(menuitem),
							 "compare-func"));
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(model),
							 func, NULL, NULL);
}

/**
 * 设置群组成员树(member-tree)的排序方式.
 * @param menuitem radio-menu-item
 * @param model member-model
 */
void DialogGroup::SetMemberTreeSortType(GtkWidget *menuitem, GtkTreeModel *model)
{
	GtkSortType type;

	if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
		return;
	type = (GtkSortType)GPOINTER_TO_INT(g_object_get_data(G_OBJECT(menuitem),
								 "sort-type"));
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model),
			 GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, type);
}

/**
 * 拖拽事件响应函数.
 * @param dlggrp 对话框类
 * @param context the drag context
 * @param x where the drop happened
 * @param y where the drop happened
 * @param data the received data
 * @param info the info that has been registered with the target in the GtkTargetList
 * @param time the timestamp at which the data was received
 */
void DialogGroup::DragDataReceived(DialogGroup *dlggrp, GdkDragContext *context,
				 gint x, gint y, GtkSelectionData *data,
				 guint info, guint time)
{
	GtkWidget *widget;
	GSList *list;

	if (data->length <= 0 || data->format != 8) {
		gtk_drag_finish(context, FALSE, FALSE, time);
		return;
	}

	list = selection_data_get_path(data);	//获取所有文件
	dlggrp->AttachEnclosure(list);
	g_slist_foreach(list, GFunc(g_free), NULL);
	g_slist_free(list);
	widget = GTK_WIDGET(g_datalist_get_data(&dlggrp->widset,
				 "enclosure-frame-widget"));
	gtk_widget_show(widget);

	gtk_drag_finish(context, TRUE, FALSE, time);
}

/**
 * 弹出选择项的菜单.
 * @param treeview tree-view
 * @param event event
 * @return Gtk+库所需
 */
gboolean DialogGroup::PopupPickMenu(GtkWidget *treeview, GdkEventButton *event)
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
 * 群组成员树(member-tree)项被激活.
 * @param treeview tree-view
 * @param path path
 * @param column column
 */
void DialogGroup::MembertreeItemActivated(GtkWidget *treeview, GtkTreePath *path,
							 GtkTreeViewColumn *column)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	PalInfo *pal;
	GroupInfo *grpinf;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, 3, &pal, -1);
	if ( (grpinf = cthrd.GetPalRegularItem(pal))) {
		if ( (grpinf->dialog))
			gtk_window_present(GTK_WINDOW(grpinf->dialog));
		else
			DialogPeer::PeerDialogEntry(grpinf);
	}
}

/**
 * 添加常规文件附件.
 * @param dlggrp 对话框类
 */
void DialogGroup::AttachRegular(DialogGroup *dlggrp)
{
	GtkWidget *widget;
	GSList *list;

	if (!(list = dlggrp->PickEnclosure(IPMSG_FILE_REGULAR)))
		return;
	dlggrp->AttachEnclosure(list);
	g_slist_foreach(list, GFunc(g_free), NULL);
	g_slist_free(list);
	widget = GTK_WIDGET(g_datalist_get_data(&dlggrp->widset,
					 "enclosure-frame-widget"));
	gtk_widget_show(widget);
}

/**
 * 添加目录文件附件.
 * @param dlggrp 对话框类
 */
void DialogGroup::AttachFolder(DialogGroup *dlggrp)
{
	GtkWidget *widget;
	GSList *list;

	if (!(list = dlggrp->PickEnclosure(IPMSG_FILE_DIR)))
		return;
	dlggrp->AttachEnclosure(list);
	g_slist_foreach(list, GFunc(g_free), NULL);
	g_slist_free(list);
	widget = GTK_WIDGET(g_datalist_get_data(&dlggrp->widset,
					 "enclosure-frame-widget"));
	gtk_widget_show(widget);
}

/**
 * 清空聊天历史记录缓冲区.
 * @param dlggrp 对话框类
 */
void DialogGroup::ClearHistoryBuffer(DialogGroup *dlggrp)
{
	dlggrp->ClearHistoryTextView();
}

/**
 * 发送消息.
 * @param dlggrp 对话框类
 */
void DialogGroup::SendMessage(DialogGroup *dlggrp)
{
	dlggrp->SendEnclosureMsg();
	dlggrp->SendTextMsg();
}

/**
 * 主对话框位置&大小改变的响应处理函数.
 * @param window 主窗口
 * @param event the GdkEventConfigure which triggered this signal
 * @param dtset data set
 * @return Gtk+库所需
 */
gboolean DialogGroup::WindowConfigureEvent(GtkWidget *window,
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
void DialogGroup::PanedDivideChanged(GtkWidget *paned, GParamSpec *pspec,
							 GData **dtset)
{
	const gchar *identify;
	gint position;

	identify = (const gchar *)g_object_get_data(G_OBJECT(paned), "position-name");
	position = gtk_paned_get_position(GTK_PANED(paned));
	g_datalist_set_data(dtset, identify, GINT_TO_POINTER(position));
}

/**
 * 对话框窗口被摧毁的响应函数.
 * @param dlggrp 对话框类
 */
void DialogGroup::DialogGroupDestroy(DialogGroup *dlggrp)
{
	delete dlggrp;
}
