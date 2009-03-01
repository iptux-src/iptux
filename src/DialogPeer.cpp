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
#include "MainWindow.h"
#include "my_chooser.h"
#include "baling.h"
#include "support.h"
#include "output.h"
#include "utils.h"

 DialogPeer::DialogPeer(gpointer data): dialog(NULL), focus(NULL),
scroll(NULL), accel(NULL), pal((Pal *) data)
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
	gchar *title;

	title = g_strdup_printf(_("Communicate with %s"), pal->NameQuote());
	dialog = create_window(title, 165, 120);
	g_free(title);

	widget_enable_dnd_uri(dialog);
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
	GtkWidget *Menu_Area_vbox;
	GtkWidget *Chat_Info_hpaned;
	GtkWidget *History_Send_vpaned;

	Menu_Area_vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(dialog), Menu_Area_vbox);
	gtk_box_pack_start(GTK_BOX(Menu_Area_vbox), 
				CreateMenuBar(), FALSE, FALSE, 0);

	Chat_Info_hpaned = gtk_hpaned_new();
	gtk_paned_set_position(
				GTK_PANED(Chat_Info_hpaned), GINT(ctr.pix * 105));
	gtk_box_pack_end(GTK_BOX(Menu_Area_vbox), 
				Chat_Info_hpaned, TRUE, TRUE, 0);
	CreateInfoArea(Chat_Info_hpaned);

	History_Send_vpaned = gtk_vpaned_new();
	gtk_paned_set_position(
				GTK_PANED(History_Send_vpaned), GINT(ctr.pix * 67));
	gtk_paned_pack1(GTK_PANED(Chat_Info_hpaned), 
				History_Send_vpaned, TRUE, TRUE);

	CreateRecordArea(History_Send_vpaned);
	CreateInputArea(History_Send_vpaned);
	gtk_widget_show_all(dialog);
}

void DialogPeer::CreateInfoArea(GtkWidget *Chat_Info_hpaned)
{
	GtkWidget *view, *frame;

	frame = create_frame(_("Pal's Info."));
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
	gtk_paned_pack2(GTK_PANED(Chat_Info_hpaned), frame, FALSE, TRUE);

	GtkWidget* Info_alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_alignment_set_padding (GTK_ALIGNMENT (Info_alignment), 0, 0, 0, 0);
	gtk_container_add (GTK_CONTAINER (frame), Info_alignment);
	gtk_alignment_set_padding (GTK_ALIGNMENT (Info_alignment),
				2, 2, 2, 2);

	GtkWidget* Info_scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (
				GTK_SCROLLED_WINDOW (Info_scrolledwindow), 
				GTK_POLICY_NEVER, 
				GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (Info_alignment), Info_scrolledwindow);
	gtk_scrolled_window_set_shadow_type (
				GTK_SCROLLED_WINDOW (Info_scrolledwindow), GTK_SHADOW_IN);

	view = create_text_view();
	gtk_container_add (GTK_CONTAINER (Info_scrolledwindow), view);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(view), infobuf);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_NONE);
	gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW (view), 2);
	gtk_text_view_set_pixels_below_lines (GTK_TEXT_VIEW (view), 2);
	gtk_text_view_set_left_margin (GTK_TEXT_VIEW (view), 5);
	gtk_text_view_set_right_margin (GTK_TEXT_VIEW (view), 5);
	FillPalInfoToBuffer(pal, infobuf);
}

void DialogPeer::CreateRecordArea(GtkWidget * paned)
{
	GtkWidget *frame, *sw;

	frame = create_frame(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);  
	gtk_paned_pack1(GTK_PANED(paned), frame, TRUE, TRUE);

	GtkWidget* Rec_alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_container_add (GTK_CONTAINER (frame), Rec_alignment);
	gtk_alignment_set_padding (GTK_ALIGNMENT (Rec_alignment), 0, 0, 0, 0);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (Rec_alignment), sw);
	gtk_scrolled_window_set_policy (
				GTK_SCROLLED_WINDOW (sw), 
				GTK_POLICY_NEVER, 
				GTK_POLICY_ALWAYS);

	scroll = create_text_view();
	gtk_container_add (GTK_CONTAINER(sw), scroll);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(scroll), FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(scroll), FALSE);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(scroll), pal->RecordQuote());
	gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW (scroll), 2);
	gtk_text_view_set_pixels_below_lines (GTK_TEXT_VIEW (scroll), 2);
	gtk_text_view_set_left_margin (GTK_TEXT_VIEW (scroll), 5);
	gtk_text_view_set_right_margin (GTK_TEXT_VIEW (scroll), 5);

	pal->ViewScroll();
}

void DialogPeer::CreateInputArea(GtkWidget * paned)
{
	extern Control ctr;
	GtkWidget *frame, *sw;
	GtkWidget *vbox, *hbb, *button;

	frame = create_frame(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);	
	gtk_paned_pack2(GTK_PANED(paned), frame, FALSE, TRUE);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	GtkWidget* Input_alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_alignment_set_padding (GTK_ALIGNMENT (Input_alignment), 0, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(vbox), Input_alignment, TRUE, TRUE, 0);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (Input_alignment), sw);
	gtk_scrolled_window_set_policy (
				GTK_SCROLLED_WINDOW (sw), 
				GTK_POLICY_AUTOMATIC, 
				GTK_POLICY_AUTOMATIC);

	focus = create_text_view();
	gtk_container_add(GTK_CONTAINER(sw), focus);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (focus), GTK_WRAP_CHAR);
	gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW (focus), 2);
	gtk_text_view_set_pixels_below_lines (GTK_TEXT_VIEW (focus), 2);
	gtk_text_view_set_left_margin (GTK_TEXT_VIEW (focus), 5);
	gtk_text_view_set_right_margin (GTK_TEXT_VIEW (focus), 5);
	gtk_drag_dest_add_uri_targets(focus);
	g_signal_connect(focus, "drag-data-received",
			 G_CALLBACK(DragPicReceived),
			 gtk_text_view_get_buffer(GTK_TEXT_VIEW(focus)));
	gtk_widget_grab_focus(focus);

	hbb = create_button_box(FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), hbb, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbb), 5);
	gtk_box_set_spacing (GTK_BOX (hbb), 10);

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
}

GtkWidget *DialogPeer::CreateMenuBar()
{
	GtkWidget *menu_bar;

	menu_bar = gtk_menu_bar_new();
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

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_More"));
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(AboutIptux::MoreEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
}

bool DialogPeer::CheckExist(gpointer data)
{
	extern UdpData udt;
	extern MainWindow *mwp;
	GtkTreeIter iter;
	GList *tmp;
	Pal *pal;

	if ( (tmp = (GList *) udt.PalGetMsgPos(data)) ) {
		pthread_mutex_lock(udt.MutexQuote());
		g_queue_delete_link(udt.MsgqueueQuote(), tmp);
		pthread_mutex_unlock(udt.MutexQuote());
	}
	if (mwp->PalGetModelIter(data, &iter))
		mwp->MakeItemBlinking(&iter, false);

	pal = (Pal *) data;
	if (pal->DialogQuote()) {
		gtk_window_present(GTK_WINDOW(pal->DialogQuote()->dialog));
		return true;
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
		snprintf(buf, MAX_BUF, _("OS: Microsoft\n"));
	else
		snprintf(buf, MAX_BUF, _("OS: Unix/Linux\n"));
	gtk_text_buffer_insert(buffer, &iter, buf, -1);

	snprintf(buf, MAX_BUF, _("System Encode: %s\n"), pal->EncodeQuote());
	gtk_text_buffer_insert(buffer, &iter, buf, -1);

	snprintf(buf, MAX_BUF, _("Sign:\n"));
	gtk_text_buffer_insert(buffer, &iter, buf, -1);
	gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
			pal->SignQuote() ? pal->SignQuote() : _("(lazy)"),
			-1, "sign", NULL);

	if (!sad || !pal->AdQuote()
		 || !(pixbuf = gdk_pixbuf_new_from_file(pal->AdQuote(), NULL)))
		return;
	snprintf(buf, MAX_BUF, _("\nAD: \n"));
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
	char ipstr[INET_ADDRSTRLEN];
	GSList *list;
	Pal *pal;

	if (select->length <= 0 || select->format != 8) {
		gtk_drag_finish(context, FALSE, FALSE, time);
		return;
	}

	pal = (Pal *) data;
	list = selection_data_get_path(select);
	sfl.SendFileInfo(list, pal);
	g_slist_free(list);	//他处释放

	inet_ntop(AF_INET, &pal->Ipv4Quote(), ipstr, INET_ADDRSTRLEN);
	pop_info(pal->DialogQuote() ? pal->DialogQuote()->dialog : inter.window,
		 pal->DialogQuote() ? pal->DialogQuote()->focus : NULL,
		 _("File Sent to \n%s[%s] "),
		 pal->NameQuote(), ipstr);

	gtk_drag_finish(context, TRUE, FALSE, time);
}

void DialogPeer::AskSharedFiles(gpointer data)
{
	extern struct interactive inter;
	Command cmd;

	cmd.SendAskShared(inter.udpsock, data, 0, NULL);
}

void DialogPeer::DragPicReceived(GtkWidget * view, GdkDragContext * context,
				 gint x, gint y, GtkSelectionData * select,
				 guint info, guint time, GtkTextBuffer * buffer)
{
	GdkPixbuf *pixbuf;
	GtkTextIter iter;
	GSList *list, *tmp;
	gint position;

	if (select->length <= 0 || select->format != 8) {
		gtk_drag_finish(context, FALSE, FALSE, time);
		return;
	}

	tmp = list = selection_data_get_path(select);
	while (tmp) {
		if( (pixbuf = gdk_pixbuf_new_from_file(
					(char *) tmp->data, NULL)) ) {
			g_object_get(buffer, "cursor-position",
							 &position, NULL);
			gtk_text_buffer_get_iter_at_offset(buffer,
							 &iter, position);
			gtk_text_buffer_insert_pixbuf(buffer,
							 &iter, pixbuf);
			g_object_unref(pixbuf);
		}
		tmp = tmp->next;
	}
	g_slist_foreach(list, GFunc(remove_foreach),
				GINT_TO_POINTER(UNKNOWN));
	g_slist_free(list);

	gtk_drag_finish(context, TRUE, FALSE, time);
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
        /*
        //空信息，不发送就是了，不用大惊小怪
		pop_warning(peer->dialog, peer->focus,
				_("<span weight=\"heavy\" underline=\"error\">"
				  "\nCan't send an empty message!!</span>"));
		*/
        return;
	}

	buf[0] = '\0', chiplist = NULL, iter = piter = start;
	do {
		if ( (pixbuf = gtk_text_iter_get_pixbuf(&iter)) ) {
			ptr = gtk_text_buffer_get_text(buffer, &piter,
								&iter, FALSE);
			snprintf(buf + strlen(buf), MAX_UDPBUF - strlen(buf),
						 "%s%c", ptr, OCCUPY_OBJECT);
			free(ptr);
			piter = iter;		//移动 piter 到新位置
			ptr = g_strdup_printf("%s" IPTUX_PATH "/%" PRIx32,
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
