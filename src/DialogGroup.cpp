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
#include "AboutIptux.h"
#include "UdpData.h"
#include "Control.h"
#include "Command.h"
#include "Pal.h"
#include "Log.h"
#include "support.h"
#include "baling.h"
#include "output.h"
#include "utils.h"

GtkWidget *DialogGroup::dialog = NULL;
 DialogGroup::DialogGroup():pal_view(NULL), record(NULL),
input(NULL), accel(NULL), group_model(NULL)
{
}

DialogGroup::~DialogGroup()
{
	g_object_unref(accel);
	g_object_unref(group_model);
}

void DialogGroup::DialogEntry()
{
	DialogGroup *dg;

	if (DialogGroup::CheckExist())
		return;
	dg = new DialogGroup;
	dg->InitDialog();
	dg->CreateDialog();
}

void DialogGroup::InitDialog()
{
	group_model = CreateGroupModel();//Pal tree model
	InitGroupModel();
}

void DialogGroup::CreateDialog()
{
	extern Control ctr;
	GtkWidget *hpaned, *vpaned;
	GtkWidget *vbox;

	dialog = create_window(_("Group Message"), 141, 138);
	accel = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(dialog), accel);
	g_signal_connect_swapped(dialog, "destroy",
				 G_CALLBACK(DialogDestroy), this);

	vbox = gtk_vbox_new(FALSE, 5);
    gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(dialog), vbox);
	gtk_box_pack_start(GTK_BOX(vbox), CreateMenuBar(), FALSE, FALSE, 0);

	hpaned = create_paned(FALSE);
	gtk_paned_set_position(GTK_PANED(hpaned), GINT(ctr.pix * 40));
	gtk_box_pack_start(GTK_BOX(vbox), hpaned, TRUE, TRUE, 0);
	CreateChooseArea(hpaned);

	vpaned = create_paned();
	gtk_paned_set_position(GTK_PANED(vpaned), GINT(ctr.pix * 91));
	gtk_paned_pack2(GTK_PANED(hpaned), vpaned, TRUE, TRUE);
	CreateRecordArea(vpaned);
	CreateInputArea(vpaned);
}

GtkWidget *DialogGroup::CreateMenuBar()
{
	GtkWidget *menu_bar;

	menu_bar = gtk_menu_bar_new();
	gtk_widget_show(menu_bar);
	CreateFileMenu(menu_bar);
	CreateHelpMenu(menu_bar);

	return menu_bar;
}

void DialogGroup::CreateChooseArea(GtkWidget * paned)
{
	GtkWidget *frame, *sw;

	pal_view = CreateGroupView();
	frame = create_frame(_("Choose Pals"));
	gtk_paned_pack1(GTK_PANED(paned), frame, FALSE, TRUE);
	sw = create_scrolled_window();
	gtk_container_add(GTK_CONTAINER(frame), sw);
	gtk_container_add(GTK_CONTAINER(sw), pal_view);
}

void DialogGroup::CreateRecordArea(GtkWidget * paned)
{
	GtkWidget *frame, *sw;

	record = create_text_view();
    gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW (record), 2);
	gtk_text_view_set_pixels_below_lines (GTK_TEXT_VIEW (record), 2);
	gtk_text_view_set_left_margin (GTK_TEXT_VIEW (record), 5);
	gtk_text_view_set_right_margin (GTK_TEXT_VIEW (record), 5);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(record), FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(record), FALSE);

	frame = create_frame(NULL);
    gtk_container_set_border_width(GTK_CONTAINER(frame), 5);  
	gtk_paned_pack1(GTK_PANED(paned), frame, TRUE, TRUE);

	sw = create_scrolled_window();
	gtk_scrolled_window_set_policy (
				GTK_SCROLLED_WINDOW (sw), 
				GTK_POLICY_NEVER, 
				GTK_POLICY_ALWAYS);

	gtk_container_add(GTK_CONTAINER(frame), sw);
	gtk_container_add(GTK_CONTAINER(sw), record);
}

void DialogGroup::CreateInputArea(GtkWidget * paned)
{
	extern Control ctr;
	GtkWidget *box, *frame, *sw;
	GtkWidget *hbb, *button;

	frame = create_frame(NULL);
	gtk_paned_pack2(GTK_PANED(paned), frame, FALSE, TRUE);

	box = create_box();
	gtk_container_add(GTK_CONTAINER(frame), box);

	input = create_text_view();
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (input), GTK_WRAP_CHAR);
	gtk_text_view_set_pixels_below_lines (GTK_TEXT_VIEW (input), 2);
	gtk_text_view_set_left_margin (GTK_TEXT_VIEW (input), 5);
	gtk_text_view_set_right_margin (GTK_TEXT_VIEW (input), 5);

	sw = create_scrolled_window();
	gtk_box_pack_start(GTK_BOX(box), sw, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(sw), input);

	hbb = create_button_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbb, FALSE, FALSE, 0);
	button = create_button(_("Close"));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(gtk_widget_destroy), dialog);
	gtk_box_pack_end(GTK_BOX(hbb), button, FALSE, FALSE, 0);
	button = create_button(_("Send"));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(SendMessage), this);
	gtk_box_pack_end(GTK_BOX(hbb), button, FALSE, FALSE, 0);
	gtk_widget_add_accelerator(button, "clicked", accel, GDK_Return,
	   FLAG_ISSET(ctr.flags, 4) ? GdkModifierType(0) : GDK_CONTROL_MASK,
						      GTK_ACCEL_VISIBLE);

	gtk_widget_grab_focus(input);
}

//群发 4,0 flag,1 pixbuf,2 nickname,3 pointer
GtkTreeModel *DialogGroup::CreateGroupModel()
{
	GtkListStore *model;

	model = gtk_list_store_new(4, G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF,
					   G_TYPE_STRING, G_TYPE_POINTER);

	return GTK_TREE_MODEL(model);
}

void DialogGroup::InitGroupModel()
{
	extern UdpData udt;
	GtkTreeIter iter;
	GdkPixbuf *pixbuf;
	GSList *tmp;
	Pal *pal;

	pthread_mutex_lock(udt.MutexQuote());
	tmp = udt.PallistQuote();
	while (tmp) {
		pal = (Pal *) tmp->data;
		tmp = tmp->next;
		if (!FLAG_ISSET(pal->FlagsQuote(), 1))
			continue;
		pixbuf = pal->GetIconPixbuf();
		gtk_list_store_append(GTK_LIST_STORE(group_model), &iter);
		gtk_list_store_set(GTK_LIST_STORE(group_model), &iter, 0, TRUE,
				   1, pixbuf, 2, pal->NameQuote(), 3, pal, -1);
		if (pixbuf)
			g_object_unref(pixbuf);
	}
	pthread_mutex_unlock(udt.MutexQuote());
}

GtkWidget *DialogGroup::CreateGroupView()
{
	GtkWidget *view;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	view = gtk_tree_view_new_with_model(group_model);
	g_signal_connect_swapped(view, "button-press-event",
			 G_CALLBACK(PopupPickMenu), group_model);
	g_signal_connect(view, "row-activated",
			 G_CALLBACK(ViewItemActivated), group_model);
	gtk_widget_show(view);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("Send"));
	renderer = gtk_cell_renderer_toggle_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "active", 0, NULL);
	g_signal_connect_swapped(renderer, "toggled",
				 G_CALLBACK(ViewToggleChange), group_model);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("Pals"));
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "pixbuf", 1, NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	return view;
}

void DialogGroup::CreateFileMenu(GtkWidget * menu_bar)
{
	GtkWidget *menu;
	GtkWidget *menu_item;

	menu_item = gtk_menu_item_new_with_mnemonic(_("_File"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
	gtk_widget_show(menu_item);

	menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
	gtk_widget_show(menu);

	menu_item = gtk_menu_item_new_with_label(_("Update List"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(UpdatePalList), this);
	gtk_widget_show(menu_item);

	menu_item = gtk_tearoff_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	gtk_widget_show(menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Close"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(gtk_widget_destroy), dialog);
	gtk_widget_show(menu_item);
}

void DialogGroup::CreateHelpMenu(GtkWidget * menu_bar)
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

bool DialogGroup::CheckExist()
{
	if (dialog == NULL)
		return false;
	gtk_window_present(GTK_WINDOW(dialog));
	return true;
}

void DialogGroup::BufferInsertText(const gchar * msg)
{
	extern Control ctr;
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	char *ptr;

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(record));
	gtk_text_buffer_get_end_iter(buffer, &iter);
	ptr = getformattime("%s", ctr.myname);
	gtk_text_buffer_insert(buffer, &iter, ptr, -1);
	free(ptr);
	ptr = g_strdup_printf("%s\n", msg);
	gtk_text_buffer_insert(buffer, &iter, ptr, -1);
	g_free(ptr);
}

void DialogGroup::SendGroupMsg(const gchar * msg)
{
	extern struct interactive inter;
	Command cmd;
	GtkTreeIter iter;
	gboolean active;
	gpointer pal;

	if (!gtk_tree_model_get_iter_first(group_model, &iter))
		return;
	do {
		gtk_tree_model_get(group_model, &iter, 0, &active, 3, &pal, -1);
		if (active)
			cmd.SendGroupMsg(inter.udpsock, pal, msg);
	} while (gtk_tree_model_iter_next(group_model, &iter));
}

void DialogGroup::ViewScroll()
{
	GtkTextBuffer *buffer;
	GtkTextIter start, end;
	GtkTextMark *mark;

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(record));
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	if (gtk_text_iter_equal(&start, &end))
		return;
	mark = gtk_text_buffer_create_mark(buffer, NULL, &end, FALSE);
	gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(record), mark, 0.0, TRUE,
								     0.0, 0.0);
	gtk_text_buffer_delete_mark(buffer, mark);
}

GtkWidget *DialogGroup::CreatePopupMenu(GtkTreeModel * model)
{
	GtkWidget *menu, *menu_item;

	menu = gtk_menu_new();
	gtk_widget_show(menu);

	menu_item = gtk_menu_item_new_with_label(_("Choose All"));
	g_signal_connect_swapped(menu_item, "activate",
					 G_CALLBACK(SelectAll), model);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Reverse All"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(TurnSelect), model);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Clear Up"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(ClearAll), model);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	return menu;
}

void DialogGroup::ViewToggleChange(GtkTreeModel * model, gchar * path)
{
	GtkTreePath *treepath;
	GtkTreeIter iter;
	gboolean active;

	treepath = gtk_tree_path_new_from_string(path);
	gtk_tree_model_get_iter(model, &iter, treepath);
	gtk_tree_path_free(treepath);
	gtk_tree_model_get(model, &iter, 0, &active, -1);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, !active, -1);
}

gboolean DialogGroup::PopupPickMenu(GtkTreeModel * model,
				    GdkEventButton * event)
{
	if (event->button != 3)
		return FALSE;
	gtk_menu_popup(GTK_MENU(CreatePopupMenu(model)), NULL, NULL,
				       NULL, NULL, event->button, event->time);

	return TRUE;
}

void DialogGroup::ViewItemActivated(GtkWidget * view, GtkTreePath * path,
				      GtkTreeViewColumn * column,
				      GtkTreeModel * model)
{
	GtkTreeIter iter;
	gpointer data;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, 3, &data, -1);
	DialogPeer::DialogEntry(data);
}

void DialogGroup::SendMessage(gpointer data)
{
	extern Log mylog;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;
	DialogGroup *dg;
	char *msg;

	dg = (DialogGroup *) data;
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(dg->input));
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	if (gtk_text_iter_equal(&start, &end)) {
        /*
		//既然空消息，不发送就是，不用大惊小怪的
        pop_warning(dialog, dg->input,
			    _("<span weight=\"heavy\" underline=\"error\">"
			      "\nCan't send an empty message!!</span>"));
        */
		return;
	}
	msg = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
	gtk_text_buffer_delete(buffer, &start, &end);

	dg->BufferInsertText(msg);
	mylog.CommunicateLog(NULL, msg);
	dg->SendGroupMsg(msg);
	dg->ViewScroll();

	g_free(msg);
	gtk_widget_grab_focus(dg->input);
}

void DialogGroup::UpdatePalList(gpointer data)
{
	DialogGroup *group;

	group = (DialogGroup *) data;
	gtk_list_store_clear(GTK_LIST_STORE(group->group_model));
	group->InitGroupModel();
}

void DialogGroup::DialogDestroy(gpointer data)
{
	dialog = NULL;
	delete(DialogGroup *) data;
}

void DialogGroup::SelectAll(GtkTreeModel * model)
{
	GtkTreeIter iter;

	if (!gtk_tree_model_get_iter_first(model, &iter))
		return;
	do {
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, TRUE, -1);
	} while (gtk_tree_model_iter_next(model, &iter));
}

void DialogGroup::TurnSelect(GtkTreeModel * model)
{
	GtkTreeIter iter;
	gboolean flag;

	if (!gtk_tree_model_get_iter_first(model, &iter))
		return;
	do {
		gtk_tree_model_get(model, &iter, 0, &flag, -1);
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, !flag, -1);
	} while (gtk_tree_model_iter_next(model, &iter));
}

void DialogGroup::ClearAll(GtkTreeModel * model)
{
	GtkTreeIter iter;

	if (!gtk_tree_model_get_iter_first(model, &iter))
		return;
	do {
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, FALSE, -1);
	} while (gtk_tree_model_iter_next(model, &iter));
}
