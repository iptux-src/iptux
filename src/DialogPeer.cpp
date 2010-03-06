//
// C++ Implementation: DialogPeer
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "DialogPeer.h"
#include "ProgramData.h"
#include "CoreThread.h"
#include "MainWindow.h"
#include "LogSystem.h"
#include "Command.h"
#include "SendFile.h"
#include "HelpDialog.h"
#include "output.h"
#include "callback.h"
#include "support.h"
#include "utils.h"
extern ProgramData progdt;
extern CoreThread cthrd;
extern MainWindow mwin;
extern LogSystem lgsys;

/**
 * 类构造函数.
 * @param grp 好友群组信息
 */
DialogPeer::DialogPeer(GroupInfo *grp):widset(NULL), mdlset(NULL),
 dtset(NULL), accel(NULL), grpinf(grp)
{
	InitSublayer();
	ReadUILayout();
}

/**
 * 类析构函数.
 */
DialogPeer::~DialogPeer()
{
	WriteUILayout();
	ClearSublayer();
}

/**
 * 好友对话框入口.
 * @param grpinf 好友群组信息
 */
void DialogPeer::PeerDialogEntry(GroupInfo *grpinf)
{
	DialogPeer *dlgpr;
	GtkWidget *window, *widget;

	dlgpr = new DialogPeer(grpinf);
	window = dlgpr->CreateMainWindow();
	gtk_container_add(GTK_CONTAINER(window), dlgpr->CreateAllArea());
	gtk_widget_show_all(window);

	/* 将焦点置于文本输入框 */
	widget = GTK_WIDGET(g_datalist_get_data(&dlgpr->widset,
					 "input-textview-widget"));
	gtk_widget_grab_focus(widget);
	/* 隐藏附件窗体 */
	widget = GTK_WIDGET(g_datalist_get_data(&dlgpr->widset,
					 "enclosure-frame-widget"));
	gtk_widget_hide(widget);
	/* 从消息队列中移除 */
	pthread_mutex_lock(cthrd.GetMutex());
	if (cthrd.MsglineContainItem(grpinf)) {
		mwin.MakeItemBlinking(grpinf, FALSE);
		cthrd.PopItemFromMsgline(grpinf);
	}
	pthread_mutex_unlock(cthrd.GetMutex());

	/* delete dlgpr;//请不要这样做，此类将会在窗口被摧毁后自动释放 */
}

/**
 * 更新好友信息.
 * @param pal 好友信息
 */
void DialogPeer::UpdatePalData(PalInfo *pal)
{
	GtkWidget *textview;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;

	textview = GTK_WIDGET(g_datalist_get_data(&widset, "info-textview-widget"));
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	gtk_text_buffer_delete(buffer, &start, &end);
	FillPalInfoToBuffer(buffer, pal);
}

/**
 * 插入好友数据.
 * @param pal 好友信息
 */
void DialogPeer::InsertPalData(PalInfo *pal)
{
	//此函数暂且无须实现
}

/**
 * 删除好友数据.
 * @param pal 好友信息
 */
void DialogPeer::DelPalData(PalInfo *pal)
{
	//此函数暂且无须实现
}

/**
 * 清除本群组所有好友数据.
 */
void DialogPeer::ClearAllPalData()
{
	//此函数暂且无须实现
}

/**
 * 显示附件.
 */
void DialogPeer::ShowEnclosure()
{
	GtkWidget *widget;

	widget = GTK_WIDGET(g_datalist_get_data(&widset, "enclosure-frame-widget"));
	gtk_widget_show(widget);
}

/**
 * 添加附件.
 * @param list 文件链表
 */
void DialogPeer::AttachEnclosure(const GSList *list)
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
void DialogPeer::ScrollHistoryTextview()
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
void DialogPeer::InitSublayer()
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
void DialogPeer::ClearSublayer()
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
void DialogPeer::ReadUILayout()
{
	GConfClient *client;
	gint numeric;

	client = gconf_client_get_default();

	numeric = gconf_client_get_int(client, GCONF_PATH "/peer_window_width", NULL);
	numeric = numeric ? numeric : 570;
	g_datalist_set_data(&dtset, "window-width", GINT_TO_POINTER(numeric));
	numeric = gconf_client_get_int(client, GCONF_PATH "/peer_window_height", NULL);
	numeric = numeric ? numeric : 420;
	g_datalist_set_data(&dtset, "window-height", GINT_TO_POINTER(numeric));

	numeric = gconf_client_get_int(client,
			 GCONF_PATH "/peer_main_paned_divide", NULL);
	numeric = numeric ? numeric : 375;
	g_datalist_set_data(&dtset, "main-paned-divide", GINT_TO_POINTER(numeric));

	numeric = gconf_client_get_int(client,
			 GCONF_PATH "/peer_historyinput_paned_divide", NULL);
	numeric = numeric ? numeric : 255;
	g_datalist_set_data(&dtset, "historyinput-paned-divide",
					 GINT_TO_POINTER(numeric));

	numeric = gconf_client_get_int(client,
			 GCONF_PATH "/peer_infoenclosure_paned_divide", NULL);
	numeric = numeric ? numeric : 255;
	g_datalist_set_data(&dtset, "infoenclosure-paned-divide",
					 GINT_TO_POINTER(numeric));

	g_object_unref(client);
}

/**
 * 写出对话框的UI布局数据.
 */
void DialogPeer::WriteUILayout()
{
	GConfClient *client;
	gint numeric;

	client = gconf_client_get_default();

	numeric = GPOINTER_TO_INT(g_datalist_get_data(&dtset, "window-width"));
	gconf_client_set_int(client, GCONF_PATH "/peer_window_width", numeric, NULL);
	numeric = GPOINTER_TO_INT(g_datalist_get_data(&dtset, "window-height"));
	gconf_client_set_int(client, GCONF_PATH "/peer_window_height", numeric, NULL);

	numeric = GPOINTER_TO_INT(g_datalist_get_data(&dtset, "main-paned-divide"));
	gconf_client_set_int(client, GCONF_PATH "/peer_main_paned_divide", numeric, NULL);

	numeric = GPOINTER_TO_INT(g_datalist_get_data(&dtset,
				 "historyinput-paned-divide"));
	gconf_client_set_int(client, GCONF_PATH "/peer_historyinput_paned_divide",
								 numeric, NULL);

	numeric = GPOINTER_TO_INT(g_datalist_get_data(&dtset,
				 "infoenclosure-paned-divide"));
	gconf_client_set_int(client, GCONF_PATH "/peer_infoenclosure_paned_divide",
								 numeric, NULL);

	g_object_unref(client);
}

/**
 * 清空聊天历史记录.
 */
void DialogPeer::ClearHistoryTextView()
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
GtkWidget *DialogPeer::CreateMainWindow()
{
	char buf[MAX_BUFLEN];
	GtkWidget *window;
	gint width, height;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	snprintf(buf, MAX_BUFLEN, _("Talk with %s"), grpinf->name);
	gtk_window_set_title(GTK_WINDOW(window), buf);
	width = GPOINTER_TO_INT(g_datalist_get_data(&dtset, "window-width"));
	height = GPOINTER_TO_INT(g_datalist_get_data(&dtset, "window-height"));
	gtk_window_set_default_size(GTK_WINDOW(window), width, height);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_add_accel_group(GTK_WINDOW(window), accel);
	widget_enable_dnd_uri(window);
	g_datalist_set_data(&widset, "window-widget", window);

	grpinf->dialog = window;
	g_object_set_data(G_OBJECT(window), "session-class", this);
	g_signal_connect_swapped(window, "destroy", G_CALLBACK(DialogPeerDestroy), this);
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
GtkWidget *DialogPeer::CreateAllArea()
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
	/*/* 加入聊天历史记录&输入区域 */
	vpaned = gtk_vpaned_new();
	g_object_set_data(G_OBJECT(vpaned), "position-name",
			 (gpointer)"historyinput-paned-divide");
	position = GPOINTER_TO_INT(g_datalist_get_data(&dtset,
				 "historyinput-paned-divide"));
	gtk_paned_set_position(GTK_PANED(vpaned), position);
	gtk_paned_pack1(GTK_PANED(hpaned), vpaned, TRUE, TRUE);
	g_signal_connect(vpaned, "notify::position",
			 G_CALLBACK(PanedDivideChanged), &dtset);
	gtk_paned_pack1(GTK_PANED(vpaned), CreateHistoryArea(), TRUE, TRUE);
	gtk_paned_pack2(GTK_PANED(vpaned), CreateInputArea(), FALSE, TRUE);
	/* 加入好友信息&附件区域 */
	vpaned = gtk_vpaned_new();
	g_object_set_data(G_OBJECT(vpaned), "position-name",
			 (gpointer)"infoenclosure-paned-divide");
	position = GPOINTER_TO_INT(g_datalist_get_data(&dtset,
				 "infoenclosure-paned-divide"));
	gtk_paned_set_position(GTK_PANED(vpaned), position);
	gtk_paned_pack2(GTK_PANED(hpaned), vpaned, FALSE, TRUE);
	g_signal_connect(vpaned, "notify::position",
			 G_CALLBACK(PanedDivideChanged), &dtset);
	gtk_paned_pack1(GTK_PANED(vpaned), CreateInfoArea(), TRUE, TRUE);
	gtk_paned_pack2(GTK_PANED(vpaned), CreateEnclosureArea(), FALSE, TRUE);

	return box;
}

/**
 * 创建菜单条.
 * @return 菜单条
 */
GtkWidget *DialogPeer::CreateMenuBar()
{
	GtkWidget *menubar;

	menubar = gtk_menu_bar_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), CreateFileMenu());
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), CreateToolMenu());
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), CreateHelpMenu());

	return menubar;
}

/**
 * 创建好友信息区域.
 * @return 主窗体
 */
GtkWidget *DialogPeer::CreateInfoArea()
{
	GtkWidget *frame, *sw;
	GtkWidget *widget;
	GtkTextBuffer *buffer;

	frame = gtk_frame_new(_("Info."));
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
	sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
		 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
						 GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(frame), sw);

	buffer = gtk_text_buffer_new(progdt.table);
	if (grpinf->member)
		FillPalInfoToBuffer(buffer, (PalInfo *)grpinf->member->data);
	widget = gtk_text_view_new_with_buffer(buffer);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(widget), FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(widget), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(widget), GTK_WRAP_NONE);
	gtk_container_add(GTK_CONTAINER(sw), widget);
	g_datalist_set_data(&widset, "info-textview-widget", widget);

	return frame;
}

/**
 * 创建附件区域.
 * @return 主窗体
 */
GtkWidget *DialogPeer::CreateEnclosureArea()
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
 * @return 主窗体
 */
GtkWidget *DialogPeer::CreateHistoryArea()
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
GtkWidget *DialogPeer::CreateInputArea()
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
			 G_CALLBACK(DragPicReceived), this);
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
 * 附件树(enclosure-tree)底层数据结构.
 * 2,0 logo,1 path \n
 * 文件图标;文件路径
 * @return enclosure-model
 */
GtkTreeModel *DialogPeer::CreateEnclosureModel()
{
	GtkListStore *model;

	model = gtk_list_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_STRING);

	return GTK_TREE_MODEL(model);
}

/**
 * 创建附件树(enclosure-tree).
 * @param model enclosure-model
 * @return 附件树
 */
GtkWidget *DialogPeer::CreateEnclosureTree(GtkTreeModel *model)
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
GtkWidget *DialogPeer::CreateFileMenu()
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

	menuitem = gtk_menu_item_new_with_label(_("Request Shared Resources"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect_swapped(menuitem, "activate",
			 G_CALLBACK(AskSharedFiles), grpinf);
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

	return menushell;
}

/**
 * 创建工具菜单.
 * @return 菜单
 */
GtkWidget *DialogPeer::CreateToolMenu()
{
	GtkWidget *menushell;
	GtkWidget *menu, *menuitem;

	menushell = gtk_menu_item_new_with_mnemonic(_("_Tools"));
	menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menushell), menu);

	menuitem = gtk_menu_item_new_with_label(_("Insert Picture"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect_swapped(menuitem, "activate", G_CALLBACK(InsertPicture), this);

	menuitem = gtk_menu_item_new_with_label(_("Clear Buffer"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect_swapped(menuitem, "activate",
			 G_CALLBACK(ClearHistoryBuffer), this);

	return menushell;
}

/**
 * 创建帮助菜单.
 * @return 菜单
 */
GtkWidget *DialogPeer::CreateHelpMenu()
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
 * 将好友信息数据写入指定的缓冲区.
 * @param buffer text-buffer
 * @param pal class PalInfo
 */
void DialogPeer::FillPalInfoToBuffer(GtkTextBuffer *buffer, PalInfo *pal)
{
	char buf[MAX_BUFLEN], ipstr[INET_ADDRSTRLEN];
	GdkPixbuf *pixbuf;
	GtkTextIter iter;

	gtk_text_buffer_get_end_iter(buffer, &iter);

	snprintf(buf, MAX_BUFLEN, _("Version: %s\n"), pal->version);
	gtk_text_buffer_insert(buffer, &iter, buf, -1);

	if (pal->group && *pal->group != '\0')
		snprintf(buf, MAX_BUFLEN, _("Nickname: %s@%s\n"), pal->name, pal->group);
	else
		snprintf(buf, MAX_BUFLEN, _("Nickname: %s\n"), pal->name);
	gtk_text_buffer_insert(buffer, &iter, buf, -1);

	snprintf(buf, MAX_BUFLEN, _("User: %s\n"), pal->user);
	gtk_text_buffer_insert(buffer, &iter, buf, -1);

	snprintf(buf, MAX_BUFLEN, _("Host: %s\n"), pal->host);
	gtk_text_buffer_insert(buffer, &iter, buf, -1);

	inet_ntop(AF_INET, &pal->ipv4, ipstr, INET_ADDRSTRLEN);
	if (pal->segdes && *pal->segdes != '\0')
		snprintf(buf, MAX_BUFLEN, _("Address: %s(%s)\n"), pal->segdes, ipstr);
	else
		snprintf(buf, MAX_BUFLEN, _("Address: %s\n"), ipstr);
	gtk_text_buffer_insert(buffer, &iter, buf, -1);

	if (!FLAG_ISSET(pal->flags, 0))
		snprintf(buf, MAX_BUFLEN, _("Compatibility: Microsoft\n"));
	else
		snprintf(buf, MAX_BUFLEN, _("Compatibility: GNU/Linux\n"));
	gtk_text_buffer_insert(buffer, &iter, buf, -1);

	snprintf(buf, MAX_BUFLEN, _("System coding: %s\n"), pal->encode);
	gtk_text_buffer_insert(buffer, &iter, buf, -1);

	if (pal->sign && *pal->sign != '\0') {
		gtk_text_buffer_insert(buffer, &iter, _("Signature:\n"), -1);
		gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
				 pal->sign, -1, "sign-words", NULL);
	}

	if (pal->photo && *pal->photo != '\0'
		 && (pixbuf = gdk_pixbuf_new_from_file(pal->photo, NULL))) {
		gtk_text_buffer_insert(buffer, &iter, _("\nPhoto:\n"), -1);
		//TODO 缩放多少才合适
		pixbuf_shrink_scale_1(&pixbuf, 200, -1);
		gtk_text_buffer_insert_pixbuf(buffer, &iter, pixbuf);
		g_object_unref(pixbuf);
	}
}

/**
 * 选择附件.
 * @param fileattr 文件类型
 * @return 文件链表
 */
GSList *DialogPeer::PickEnclosure(uint32_t fileattr)
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
bool DialogPeer::SendEnclosureMsg()
{
	SendFile sfile;
	GtkWidget *frame, *treeview;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GSList *list;
	gchar *filepath;
	PalInfo *pal;

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
	if (!(grpinf->member))
		pal = cthrd.GetPalFromList(grpinf->grpid);
	else
		pal = (PalInfo *)grpinf->member->data;
	sfile.SendFileInfoEntry(pal, list);
	/* g_slist_foreach(list, GFunc(glist_delete_foreach), UNKNOWN); */
	g_slist_free(list);

	return true;
}

/**
 * 发送文本消息.
 * @return 是否发送数据
 */
bool DialogPeer::SendTextMsg()
{
	static uint32_t count = 0;
	GtkWidget *textview;
	GtkTextBuffer *buffer;
	GtkTextIter start, end, piter, iter;
	GdkPixbuf *pixbuf;
	char buf[MAX_UDPLEN];
	gchar *chipmsg, *ptr;
	pthread_t pid;
	size_t len;
	MsgPara *para;
	ChipData *chip;
	GSList *dtlist;

	/* 考察缓冲区内是否存在数据 */
	textview = GTK_WIDGET(g_datalist_get_data(&widset, "input-textview-widget"));
	gtk_widget_grab_focus(textview);	//为下一次任务做准备
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	if (gtk_text_iter_equal(&start, &end))
		return false;

	/* 一些初始化工作 */
	buf[0] = '\0';	//缓冲区数据为空
	ptr = buf;
	len = 0;
	dtlist = NULL;	//数据链表为空
	/* 获取数据 */
	piter = iter = start;	//让指针指向缓冲区开始位置
	do {
		/**
		 * @note 由于gtk_text_iter_forward_find_char()会跳过当前字符，
		 * 所以必须先考察第一个字符是否为图片.
		 */
		if ( (pixbuf = gtk_text_iter_get_pixbuf(&iter))) {
			/* 读取图片之前的字符数据，并写入缓冲区 */
			chipmsg = gtk_text_buffer_get_text(buffer, &piter, &iter, FALSE);
			snprintf(ptr, MAX_UDPLEN - len, "%s%c", chipmsg, OCCUPY_OBJECT);
			len += strlen(ptr);
			ptr = buf + len;
			g_free(chipmsg);
			piter = iter;	//移动 piter 到新位置
			/* 保存图片 */
			chipmsg = g_strdup_printf("%s" IPTUX_PATH "/%" PRIx32,
					 g_get_user_config_dir(), count++);
			gdk_pixbuf_save(pixbuf, chipmsg, "bmp", NULL, NULL);
			/* 新建一个碎片数据(图片)，并加入数据链表 */
			chip = new ChipData;
			chip->type = PICTURE_TYPE;
			chip->data = chipmsg;
			dtlist = g_slist_append(dtlist, chip);
		}
	} while (gtk_text_iter_forward_find_char(&iter,
			 GtkTextCharPredicate(giter_compare_foreach),
			 GUINT_TO_POINTER(ATOM_OBJECT), &end));
	/* 读取余下的字符数据，并写入缓冲区 */
	chipmsg = gtk_text_buffer_get_text(buffer, &piter, &end, FALSE);
	snprintf(ptr, MAX_UDPLEN - len, "%s", chipmsg);
	g_free(chipmsg);
	/* 新建一个碎片数据(字符串)，并加入数据链表 */
	chip = new ChipData;
	chip->type = STRING_TYPE;
	chip->data = g_strdup(buf);
	dtlist = g_slist_prepend(dtlist, chip);	//保证字符串先被发送

	/* 清空缓冲区并发送数据 */
	gtk_text_buffer_delete(buffer, &start, &end);
	FeedbackMsg(dtlist);
	para = PackageMsg(dtlist);
	pthread_create(&pid, NULL, ThreadFunc(ThreadSendTextMsg), para);
	pthread_detach(pid);
	/* g_slist_foreach(dtlist, GFunc(glist_delete_foreach), CHIP_DATA); */
	/* g_slist_free(dtlist); */

	return true;
}

/**
 * 回馈消息.
 * @param dtlist 数据链表
 * @note 请不要修改链表(dtlist)中的数据
 */
void DialogPeer::FeedbackMsg(const GSList *dtlist)
{
	MsgPara para;

	/* 构建消息封装包 */
	para.pal = NULL;
	para.stype = ME_TYPE;
	para.btype = grpinf->type;
	para.dtlist = (GSList *)dtlist;

	/* 交给某人处理吧 */
	cthrd.InsertMsgToGroupInfoItem(grpinf, &para);
	para.dtlist = NULL;	//防止参数数据被修改
}

/**
 * 封装消息.
 * @param dtlist 数据链表
 * @return 消息封装包
 */
MsgPara *DialogPeer::PackageMsg(GSList *dtlist)
{
	MsgPara *para;

	para = new MsgPara;
	if (!(grpinf->member))
		para->pal = cthrd.GetPalFromList(grpinf->grpid);
	else
		para->pal = (PalInfo *)grpinf->member->data;
	para->stype = ME_TYPE;
	para->btype = grpinf->type;
	para->dtlist = dtlist;

	return para;
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
void DialogPeer::DragDataReceived(DialogPeer *dlgpr, GdkDragContext *context,
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
	dlgpr->AttachEnclosure(list);
	g_slist_foreach(list, GFunc(glist_delete_foreach), GINT_TO_POINTER(UNKNOWN));
	g_slist_free(list);
	widget = GTK_WIDGET(g_datalist_get_data(&dlgpr->widset,
				 "enclosure-frame-widget"));
	gtk_widget_show(widget);

	gtk_drag_finish(context, TRUE, FALSE, time);
}

/**
 * 图片拖拽事件响应函数.
 * @param dlgpr 对话框类
 * @param context the drag context
 * @param x where the drop happened
 * @param y where the drop happened
 * @param data the received data
 * @param info the info that has been registered with the target in the GtkTargetList
 * @param time the timestamp at which the data was received
 */
void DialogPeer::DragPicReceived(DialogPeer *dlgpr, GdkDragContext *context,
				 gint x, gint y, GtkSelectionData *data,
				 guint info, guint time)
{
	GtkWidget *widget;
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	GdkPixbuf *pixbuf;
	GSList *list, *flist, *tlist;
	gint position;

	if (data->length <= 0 || data->format != 8) {
		gtk_drag_finish(context, FALSE, FALSE, time);
		return;
	}

	/* 获取(text-buffer)的当前插入点 */
	widget = GTK_WIDGET(g_datalist_get_data(&dlgpr->widset, "input-textview-widget"));
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	g_object_get(buffer, "cursor-position", &position, NULL);
	gtk_text_buffer_get_iter_at_offset(buffer, &iter, position);
	/* 分离图片文件和常规文件，图片立即处理，常规文件稍候再处理 */
	flist = NULL;	//预置常规文件链表为空
	tlist = list = selection_data_get_path(data);	//获取所有文件
	while (tlist) {
		if ( (pixbuf = gdk_pixbuf_new_from_file((char *)tlist->data, NULL))) {
			/* 既然是图片，那就立即处理吧 */
			gtk_text_buffer_insert_pixbuf(buffer, &iter, pixbuf);
			g_object_unref(pixbuf);
		} else {
			/* 将文件路径转移至文件链表(flist) */
			flist = g_slist_append(flist, tlist->data);
			tlist->data = NULL;
		}
		tlist = g_slist_next(tlist);
	}
	/*/* 释放链表数据 */
	g_slist_foreach(list, GFunc(glist_delete_foreach), GINT_TO_POINTER(UNKNOWN));
	g_slist_free(list);
	/* 如果文件链表有文件，那就添加为附件吧 */
	if (flist) {
		dlgpr->AttachEnclosure(flist);
		g_slist_foreach(flist, GFunc(glist_delete_foreach),
					 GINT_TO_POINTER(UNKNOWN));
		g_slist_free(flist);
		widget = GTK_WIDGET(g_datalist_get_data(&dlgpr->widset,
					 "enclosure-frame-widget"));
		gtk_widget_show(widget);
	}

	gtk_drag_finish(context, TRUE, FALSE, time);
}

/**
 * 添加常规文件附件.
 * @param dlgpr 对话框类
 */
void DialogPeer::AttachRegular(DialogPeer *dlgpr)
{
	GtkWidget *widget;
	GSList *list;

	if (!(list = dlgpr->PickEnclosure(IPMSG_FILE_REGULAR)))
		return;
	dlgpr->AttachEnclosure(list);
	g_slist_foreach(list, GFunc(glist_delete_foreach), GINT_TO_POINTER(UNKNOWN));
	g_slist_free(list);
	widget = GTK_WIDGET(g_datalist_get_data(&dlgpr->widset,
					 "enclosure-frame-widget"));
	gtk_widget_show(widget);
}

/**
 * 添加目录文件附件.
 * @param dlgpr 对话框类
 */
void DialogPeer::AttachFolder(DialogPeer *dlgpr)
{
	GtkWidget *widget;
	GSList *list;

	if (!(list = dlgpr->PickEnclosure(IPMSG_FILE_DIR)))
		return;
	dlgpr->AttachEnclosure(list);
	g_slist_foreach(list, GFunc(glist_delete_foreach), GINT_TO_POINTER(UNKNOWN));
	g_slist_free(list);
	widget = GTK_WIDGET(g_datalist_get_data(&dlgpr->widset,
					 "enclosure-frame-widget"));
	gtk_widget_show(widget);
}

/**
 * 请求获取此好友的共享文件.
 * @param grpinf 好友群组信息
 */
void DialogPeer::AskSharedFiles(GroupInfo *grpinf)
{
	Command cmd;
	PalInfo *pal;

	if (!(grpinf->member))
		pal = cthrd.GetPalFromList(grpinf->grpid);
	else
		pal = (PalInfo *)grpinf->member->data;

	cmd.SendAskShared(cthrd.UdpSockQuote(), pal, 0, NULL);
}

/**
 * 插入图片到输入缓冲区.
 * @param dlgpr 对话框类
 */
void DialogPeer::InsertPicture(DialogPeer *dlgpr)
{
	GtkWidget *widget, *window;
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	GdkPixbuf *pixbuf;
	gchar *filename;
	gint position;

	window = GTK_WIDGET(g_datalist_get_data(&dlgpr->widset, "window-widget"));
	if (!(filename = choose_file_with_preview(
		 _("Please select a picture to insert the buffer"), window)))
		return;

	if (!(pixbuf = gdk_pixbuf_new_from_file(filename, NULL))) {
		g_free(filename);
		return;
	}
	g_free(filename);

	widget = GTK_WIDGET(g_datalist_get_data(&dlgpr->widset,
					 "input-textview-widget"));
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	g_object_get(buffer, "cursor-position", &position, NULL);
	gtk_text_buffer_get_iter_at_offset(buffer, &iter, position);
	gtk_text_buffer_insert_pixbuf(buffer, &iter, pixbuf);
	g_object_unref(pixbuf);
}

/**
 * 清空聊天历史记录缓冲区.
 * @param dlgpr 对话框类
 */
void DialogPeer::ClearHistoryBuffer(DialogPeer *dlgpr)
{
	dlgpr->ClearHistoryTextView();
}

/**
 * 发送消息.
 * @param dlgpr 对话框类
 */
void DialogPeer::SendMessage(DialogPeer *dlgpr)
{
	dlgpr->SendEnclosureMsg();
	dlgpr->SendTextMsg();
}

/**
 * 主对话框位置&大小改变的响应处理函数.
 * @param window 主窗口
 * @param event the GdkEventConfigure which triggered this signal
 * @param dtset data set
 * @return Gtk+库所需
 */
gboolean DialogPeer::WindowConfigureEvent(GtkWidget *window,
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
void DialogPeer::PanedDivideChanged(GtkWidget *paned, GParamSpec *pspec,
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
 * @param dlgpr 对话框类
 */
void DialogPeer::DialogPeerDestroy(DialogPeer *dlgpr)
{
	delete dlgpr;
}

/**
 * 发送文本消息.
 * @param para 消息参数
 */
void DialogPeer::ThreadSendTextMsg(MsgPara *para)
{
	Command cmd;
	GSList *tlist;
	char *ptr;
	int sock;

	tlist = para->dtlist;
	while (tlist) {
		ptr = ((ChipData *)tlist->data)->data;
		switch (((ChipData *)tlist->data)->type) {
		case STRING_TYPE:
			/* 文本类型 */
			cmd.SendMessage(cthrd.UdpSockQuote(), para->pal, ptr);
			break;
		case PICTURE_TYPE:
			/* 图片类型 */
			if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
				pop_error(_("Fatal Error!!\nFailed to create new socket!"
							 "\n%s"), strerror(errno));
				exit(1);
			}
			cmd.SendSublayer(sock, para->pal, IPTUX_MSGPICOPT, ptr);
			close(sock);	//关闭网络套接口
			/*/* 删除此图片 */
			unlink(ptr);	//此文件已无用处
			break;
		default:
			break;
		}
		tlist = g_slist_next(tlist);
	}

	/* 释放资源 */
	delete para;
}
