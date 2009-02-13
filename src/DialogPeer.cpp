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
#include "SendFile.h"
#include "AboutIptux.h"
#include "UdpData.h"
#include "Control.h"
#include "Command.h"
#include "my_chooser.h"
#include "baling.h"
#include "support.h"
#include "output.h"
#include "utils.h"

 DialogPeer::DialogPeer(gpointer data):pal((Pal *) data),
dialog(NULL), focus(NULL), scroll(NULL),
accel(NULL)
{
	extern Control ctr;

	infobuf = gtk_text_buffer_new(ctr.table);
	pal->DialogQuote() = this;
}

DialogPeer::~DialogPeer()
{
	extern Control ctr;
	GtkTextIter start, end;

	pal->DialogQuote() = NULL;
	g_object_unref(infobuf);
	g_object_unref(accel);
	if (FLAG_ISSET(ctr.flags, 3)) {
		gtk_text_buffer_get_bounds(pal->RecordQuote(), &start, &end);
		gtk_text_buffer_delete(pal->RecordQuote(), &start, &end);
	}
}

void DialogPeer::DialogEntry(gpointer data)
{
	DialogPeer *peer;

	if (DialogPeer::CheckExist(data))
		return;
	peer = new DialogPeer(data);
	peer->CreateDialog();
	peer->CreateAllArea();
}

void DialogPeer::CreateDialog()
{
	GdkColor color = { 8, 39321, 41634, 65535 };
	GtkTargetEntry target = { "text/plain", 0, 0 };
	gchar *title;

	title = g_strdup_printf(_("Communicate with %s"), pal->NameQuote());
	dialog = create_window(title, 162, 111);
	g_free(title);
	gtk_widget_modify_bg(dialog, GTK_STATE_NORMAL, &color);
	gtk_drag_dest_set(dialog, GTK_DEST_DEFAULT_ALL,
				    &target, 1, GDK_ACTION_MOVE);
	g_signal_connect_swapped(dialog, "drag-data-received",
				    G_CALLBACK(DragDataReceived), pal);
	accel = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(dialog), accel);
	g_signal_connect_swapped(dialog, "destroy",
				    G_CALLBACK(DialogDestroy), this);
}

void DialogPeer::CreateAllArea()
{
	extern Control ctr;
	GtkWidget *box;
	GtkWidget *hpaned, *vpaned;

	box = create_box();
	gtk_container_add(GTK_CONTAINER(dialog), box);
	gtk_box_pack_start(GTK_BOX(box), CreateMenuBar(), FALSE, FALSE, 0);
	hpaned = create_paned(FALSE);
	gtk_paned_set_position(GTK_PANED(hpaned), GINT(ctr.pix * 107));
	gtk_box_pack_end(GTK_BOX(box), hpaned, TRUE, TRUE, 0);
	CreateInfoArea(hpaned);

	vpaned = create_paned();
	gtk_paned_set_position(GTK_PANED(vpaned), GINT(ctr.pix * 67));
	gtk_paned_pack1(GTK_PANED(hpaned), vpaned, TRUE, TRUE);
	CreateRecordArea(vpaned);
	CreateInputArea(vpaned);
}

void DialogPeer::CreateInfoArea(GtkWidget * paned)
{
	GdkColor color = { 8, 65535, 65535, 55000 };
	GtkWidget *view, *frame, *sw;

	view = create_text_view();
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(view), infobuf);
	gtk_widget_modify_base(view, GTK_STATE_NORMAL, &color);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_NONE);
	FillPalInfoToBuffer(pal, infobuf);
	frame = create_frame(_("Pal's Infomation"));
	gtk_paned_pack2(GTK_PANED(paned), frame, FALSE, TRUE);
	sw = create_scrolled_window();
	gtk_container_add(GTK_CONTAINER(frame), sw);
	gtk_container_add(GTK_CONTAINER(sw), view);
}

void DialogPeer::CreateRecordArea(GtkWidget * paned)
{
	GtkWidget *frame, *sw;

	scroll = create_text_view();
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(scroll), FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(scroll), FALSE);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(scroll), pal->RecordQuote());
	frame = create_frame(_("Chat History"));
	gtk_paned_pack1(GTK_PANED(paned), frame, TRUE, TRUE);
	sw = create_scrolled_window();
	gtk_container_add(GTK_CONTAINER(frame), sw);
	gtk_container_add(GTK_CONTAINER(sw), scroll);
	pal->ViewScroll();
}

void DialogPeer::CreateInputArea(GtkWidget * paned)
{
	extern Control ctr;
	GtkWidget *frame, *sw;
	GtkWidget *vbox, *hbb, *button;

	frame = create_frame(_("Input Your Message"));
	gtk_paned_pack2(GTK_PANED(paned), frame, FALSE, TRUE);
	vbox = create_box();
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	focus = create_text_view();
	g_signal_connect(focus, "drag-data-received",
			 G_CALLBACK(DragPicReceived),
			 gtk_text_view_get_buffer(GTK_TEXT_VIEW(focus)));
	sw = create_scrolled_window();
	gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(sw), focus);

	hbb = create_button_box(FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), hbb, FALSE, FALSE, 0);

	button = create_button(_("Close"));
	gtk_box_pack_end(GTK_BOX(hbb), button, FALSE, FALSE, 0);
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(gtk_widget_destroy), dialog);
	button = create_button(_("Send"));
	gtk_box_pack_end(GTK_BOX(hbb), button, FALSE, FALSE, 0);
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(SendMessage), this);
	gtk_widget_add_accelerator(button, "clicked", accel, GDK_Return,
	      FLAG_ISSET(ctr.flags, 4) ? GdkModifierType(0) : GDK_CONTROL_MASK,
						      GTK_ACCEL_VISIBLE);
	gtk_widget_grab_focus(focus);
}

GtkWidget *DialogPeer::CreateMenuBar()
{
	GtkWidget *menu_bar;

	menu_bar = gtk_menu_bar_new();
	update_widget_bg(menu_bar, __BACK_DIR "/title.png");
	gtk_widget_show(menu_bar);
	CreateFileMenu(menu_bar);
	CreateToolMenu(menu_bar);
	CreateHelpMenu(menu_bar);

	return menu_bar;
}

void DialogPeer::CreateFileMenu(GtkWidget * menu_bar)
{
	GtkWidget *menu;
	GtkWidget *menu_item;

	menu_item = gtk_menu_item_new_with_mnemonic(_("_File"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
	gtk_widget_show(menu_item);

	menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
	gtk_widget_show(menu);

	menu_item = gtk_menu_item_new_with_label(_("Send File"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(SendFile::SendRegular), pal);
	gtk_widget_show(menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Send Folder"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(SendFile::SendFolder), pal);
	gtk_widget_show(menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Ask For Shared Files"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(AskSharedFiles), pal);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_tearoff_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	gtk_widget_show(menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Close"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(gtk_widget_destroy), dialog);
	gtk_widget_show(menu_item);
}

void DialogPeer::CreateToolMenu(GtkWidget * menu_bar)
{
	GtkWidget *image;
	GtkWidget *menu;
	GtkWidget *menu_item;

	menu_item = gtk_menu_item_new_with_mnemonic(_("_Tools"));
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

	menu = gtk_menu_new();
	gtk_widget_show(menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

	menu_item = gtk_menu_item_new_with_label(_("Insert Picture"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(InsertPixbuf), this);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	gtk_widget_show(menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Clear Buffer"));
	g_signal_connect_swapped(menu_item, "activate",
			 G_CALLBACK(ClearRecordBuffer), pal->RecordQuote());
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	gtk_widget_show(menu_item);
}

void DialogPeer::CreateHelpMenu(GtkWidget * menu_bar)
{
	GtkWidget *menu;
	GtkWidget *menu_item;

	menu_item = gtk_menu_item_new_with_mnemonic(_("_Help"));
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

	menu = gtk_menu_new();
	gtk_widget_show(menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

	menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, accel);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(AboutIptux::AboutEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
}

bool DialogPeer::CheckExist(gpointer data)
{
	extern UdpData udt;
	GList *tmp;
	Pal *pal;

	pal = (Pal *) data;
	if (pal->DialogQuote()) {
		gtk_window_present(GTK_WINDOW(pal->DialogQuote()->dialog));
		return true;
	}
	tmp = (GList *) udt.PalGetMsgPos(data);
	if (tmp) {
		pthread_mutex_lock(udt.MutexQuote());
		g_queue_delete_link(udt.MsgqueueQuote(), tmp);
		pthread_mutex_unlock(udt.MutexQuote());
	}

	return false;
}

void DialogPeer::FillPalInfoToBuffer(gpointer data, GtkTextBuffer * buffer,
				     bool sad)
{
	extern Control ctr;
	char buf[MAX_BUF], ipstr[INET_ADDRSTRLEN];
	GdkPixbuf *pixbuf;
	GtkTextIter iter;
	Pal *pal;

	pal = (Pal *) data;
	gtk_text_buffer_get_end_iter(buffer, &iter);

	snprintf(buf, MAX_BUF, _("Version: %s\n"), pal->VersionQuote());
	gtk_text_buffer_insert(buffer, &iter, buf, -1);

	if (*pal->GroupQuote() == '\0')
		snprintf(buf, MAX_BUF, _("Nickname: %s\n"), pal->NameQuote());
	else
		snprintf(buf, MAX_BUF, _("Nickname: %s@%s\n"),
				 pal->NameQuote(), pal->GroupQuote());
	gtk_text_buffer_insert(buffer, &iter, buf, -1);

	snprintf(buf, MAX_BUF, _("User: %s\n"), pal->UserQuote());
	gtk_text_buffer_insert(buffer, &iter, buf, -1);

	snprintf(buf, MAX_BUF, _("Host: %s\n"), pal->HostQuote());
	gtk_text_buffer_insert(buffer, &iter, buf, -1);

	inet_ntop(AF_INET, &pal->Ipv4Quote(), ipstr, INET_ADDRSTRLEN);
	snprintf(buf, MAX_BUF, _("Address: %s(%s)\n"),
				 pal->SegmentQuote(), ipstr);
	gtk_text_buffer_insert(buffer, &iter, buf, -1);

	if (!FLAG_ISSET(pal->FlagsQuote(), 0))
		snprintf(buf, MAX_BUF, _("Compatibility: Microsoft\n"));
	else
		snprintf(buf, MAX_BUF, _("Compatibility: GNU/Linux\n"));
	gtk_text_buffer_insert(buffer, &iter, buf, -1);

	snprintf(buf, MAX_BUF, _("System Encode: %s\n"), pal->EncodeQuote());
	gtk_text_buffer_insert(buffer, &iter, buf, -1);

	snprintf(buf, MAX_BUF, _("Personal Signature:\n"));
	gtk_text_buffer_insert(buffer, &iter, buf, -1);
	gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
			pal->SignQuote() ? pal->SignQuote() : _("(lazy)"),
			-1, "sign", NULL);

	if (!sad || !pal->AdQuote()
		 || !(pixbuf = gdk_pixbuf_new_from_file(pal->AdQuote(), NULL)))
		return;
	snprintf(buf, MAX_BUF, _("\nAdvertisement: \n"));
	gtk_text_buffer_insert(buffer, &iter, buf, -1);
	pixbuf_shrink_scale_1(&pixbuf, GINT(ctr.pix * 51), -1);
	gtk_text_buffer_insert_pixbuf(buffer, &iter, pixbuf);
	g_object_unref(pixbuf);
}

void DialogPeer::DragDataReceived(gpointer data, GdkDragContext * context,
				  gint x, gint y, GtkSelectionData * select,
				  guint info, guint time)
{
	extern SendFile sfl;
	extern struct interactive inter;
	const char *prl = "file://";
	char ipstr[INET_ADDRSTRLEN], *tmp, *file;
	GSList *list;
	Pal *pal;

	if (select->length <= 0 || select->format != 8
		   || strcasestr((char *)select->data, prl) == NULL) {
		gtk_drag_finish(context, FALSE, FALSE, time);
		return;
	}

	list = NULL, tmp = (char *)select->data;
	while (tmp = strcasestr(tmp, prl)) {
		file = my_getline(tmp + strlen(prl));
		list = g_slist_append(list, file);
		tmp += strlen(prl) + strlen(file);
	}
	pal = (Pal *) data;
	sfl.SendFileInfo(list, pal);
	g_slist_free(list);	//他处释放

	inet_ntop(AF_INET, &pal->Ipv4Quote(), ipstr, INET_ADDRSTRLEN);
	pop_info(pal->DialogQuote() ? pal->DialogQuote()->dialog : inter.window,
		 pal->DialogQuote() ? pal->DialogQuote()->focus : NULL,
		 _("Sending the files' infomation to \n%s[%s] is done!"),
		 pal->NameQuote(), ipstr);

	gtk_drag_finish(context, TRUE, FALSE, time);
}

void DialogPeer::AskSharedFiles(gpointer data)
{
	extern struct interactive inter;
	Command cmd;

	cmd.SendAskShared(inter.udpsock, data);
}

void DialogPeer::DragPicReceived(GtkWidget * view, GdkDragContext * context,
				 gint x, gint y, GtkSelectionData * select,
				 guint info, guint time, GtkTextBuffer * buffer)
{
	const char *prl = "file://";
	GdkPixbuf *pixbuf;
	GtkTextIter iter;
	char *tmp, *file;
	gboolean flag;
	gint position;

	if (select->length <= 0 || select->format != 8
		   || strcasestr((char *)select->data, prl) == NULL) {
		gtk_drag_finish(context, FALSE, FALSE, time);
		return;
	}

	flag = FALSE, tmp = (char *)select->data;
	while (tmp = strcasestr(tmp, prl)) {
		file = my_getline(tmp + strlen(prl));
		pixbuf = gdk_pixbuf_new_from_file(file, NULL);
		if (pixbuf) {
			g_object_get(buffer, "cursor-position", &position, NULL);
			gtk_text_buffer_get_iter_at_offset(buffer, &iter, position);
			gtk_text_buffer_insert_pixbuf(buffer, &iter, pixbuf);
			g_object_unref(pixbuf);
			flag = TRUE;
		}
		tmp += strlen(prl) + strlen(file);
		free(file);
	}

	gtk_drag_finish(context, flag, FALSE, time);
	if (flag)
		g_signal_stop_emission_by_name(view, "drag-data-received");
}

void DialogPeer::DialogDestroy(gpointer data)
{
	delete(DialogPeer *) data;
}

void DialogPeer::InsertPixbuf(gpointer data)
{
	GdkPixbuf *pixbuf;
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	DialogPeer *peer;
	gchar *filename;
	gint position;

	peer = (DialogPeer *) data;
	if (!(filename = my_chooser:: choose_file(
			     _("Please choose a picture to insert the buffer"),
			     peer->dialog)))
		return;

	pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
	g_free(filename);
	if (!pixbuf)
		return;
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(peer->focus));
	g_object_get(buffer, "cursor-position", &position, NULL);
	gtk_text_buffer_get_iter_at_offset(buffer, &iter, position);
	gtk_text_buffer_insert_pixbuf(buffer, &iter, pixbuf);
	g_object_unref(pixbuf);
}

void DialogPeer::ClearRecordBuffer(GtkTextBuffer * buffer)
{
	GtkTextIter start, end;

	gtk_text_buffer_get_bounds(buffer, &start, &end);
	if (!gtk_text_iter_equal(&start, &end))
		gtk_text_buffer_delete(buffer, &start, &end);
}

void DialogPeer::SendMessage(gpointer data)
{
	static uint32_t count = 0;
	char buf[MAX_UDPBUF], *ptr;
	struct sendmsg_para *para;
	DialogPeer *peer;
	GtkTextIter start, end, piter, iter;
	GtkTextBuffer *buffer;
	GdkPixbuf *pixbuf;
	GSList *chiplist;

	peer = (DialogPeer *) data;
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(peer->focus));
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	if (gtk_text_iter_equal(&start, &end)) {
		pop_warning(peer->dialog, peer->focus,
			    _("<span weight=\"heavy\" underline=\"error\">"
			      "\nCan't send an empty message!!</span>"));
		return;
	}

	buf[0] = '\0', chiplist = NULL, iter = piter = start;
	do {
		if (pixbuf = gtk_text_iter_get_pixbuf(&iter)) {
			ptr = gtk_text_buffer_get_text(buffer, &piter,
								&iter, FALSE);
			snprintf(buf + strlen(buf), MAX_UDPBUF - strlen(buf),
						 "%s%c", ptr, OCCUPY_OBJECT);
			free(ptr);
			piter = iter;		//移动 piter 到新位置
			ptr = g_strdup_printf("%s/iptux/%x",
					    g_get_user_config_dir(), count++);
			gdk_pixbuf_save(pixbuf, ptr, "bmp", NULL, NULL);
			chiplist = g_slist_append(chiplist,
					   new ChipData(PICTURE, ptr));
		}
	} while (gtk_text_iter_forward_find_char(&iter,
				     GtkTextCharPredicate(compare_foreach),
				     GUINT_TO_POINTER(ATOM_OBJECT), &end));
	ptr = gtk_text_buffer_get_text(buffer, &piter, &iter, FALSE);
	snprintf(buf + strlen(buf), MAX_UDPBUF - strlen(buf), "%s", ptr);
	free(ptr);
	chiplist = g_slist_prepend(chiplist, new ChipData(STRING, Strdup(buf)));

	gtk_text_buffer_delete(buffer, &start, &end);
	peer->pal->BufferInsertData(chiplist, SELF);
	gtk_widget_grab_focus(peer->focus);

	para = (struct sendmsg_para *)Malloc(sizeof(struct sendmsg_para));
	para->data = peer->pal, para->chiplist = chiplist;
	thread_create(ThreadFunc(ThreadSendMessage), para, false);
}

void DialogPeer::ThreadSendMessage(gpointer data)
{
	extern struct interactive inter;
	struct sendmsg_para *para;
	Command cmd;
	GSList *tmp;
	char *ptr;
	int sock;

	para = (struct sendmsg_para *)data;
	tmp = para->chiplist;
	while (tmp) {
		ptr = ((ChipData *) tmp->data)->data;
		switch (((ChipData *) tmp->data)->type) {
		case STRING:
			cmd.SendMessage(inter.udpsock, para->data, ptr);
			break;
		case PICTURE:
			sock = Socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
			cmd.SendSublayer(sock, para->data, IPTUX_MSGPICOPT, ptr);
			close(sock), unlink(ptr);
			break;
		default:
			break;
		}
		tmp = tmp->next;
	}

	g_slist_foreach(para->chiplist, GFunc(remove_foreach),
					GINT_TO_POINTER(CHIPDATA));
	g_slist_free(para->chiplist);
	free(para);
}
