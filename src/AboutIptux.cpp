//
// C++ Implementation: AboutIptux
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//          Jally <jallyx@163.com> & ManPT <pentie@gmail.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "AboutIptux.h"
#include "baling.h"

GtkWidget *AboutIptux::about = NULL;
GtkWidget *AboutIptux::more = NULL;
AboutIptux::AboutIptux()
{
}

AboutIptux::~AboutIptux()
{
}

void AboutIptux::AboutEntry()
{
	AboutIptux ai;

	if (AboutIptux::CheckExist(about))
		return;
	ai.CreateAbout();
	ai.RunDialog(&about);
}

void AboutIptux::MoreEntry()
{
	AboutIptux ai;

	if (AboutIptux::CheckExist(more))
		return;
	ai.CreateMore();
	ai.RunDialog(&more);
}

void AboutIptux::CreateAbout()
{
	const char *authors[] = {
		"Jally\tjallyx@163.com",
		NULL
	};
	const char *artists[] = {
		"ManPT\tpentie@gmail.com",
		"Jally\t\tjallyx@163.com",
		NULL
	};
	const char *translators = _("LiJinhui\nLiuTao");
	GdkPixbuf *pixbuf;

	about = gtk_about_dialog_new();
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about), _("iptux"));
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about), "0.4.5");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about),
				       "Copyright © 2008-2009 by Jally");
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about),
				      _("A GTK+ based LAN Messenger."));
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about),
				     "http://code.google.com/p/iptux/");
	gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(about), "GPL 2+");
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about), authors);
	gtk_about_dialog_set_artists(GTK_ABOUT_DIALOG(about), artists);
	gtk_about_dialog_set_translator_credits(GTK_ABOUT_DIALOG(about),
								translators);
	pixbuf = gdk_pixbuf_new_from_file(__LOGO_PATH "/ip-tux.png", NULL);
	if (pixbuf) {
		gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(about), pixbuf);
		g_object_unref(pixbuf);
	}
}

void AboutIptux::CreateMore()
{
	char *labels[] = {_("Help"), _("Contributers"), _("..."), NULL};
	char *text[] = {
		_("Project Home: \nhttp://code.google.com/p/iptux/\n\n"
		  "User and Developer Group: \nhttps://groups.google.com/group/iptux/\n\n"
		  "Note that you can get help form the project wiki page.\n\n"
		  "If you find no solutions in any of the existed documents, "
		  "feel free to drop a email at iptux@googlegroups.com, "
		  "lots of users and developers would be glade to help about your problems."),
		_("It's an honer that iptux was contributed voluntarilly by many people. "
		  "Without your help, iptux could never make it.\n\n"
		  "Especially, Here's some we would like to thank much:\n\n"
		  "<ChenGang>\n"
		  "<liangsuilong@gmail.com>\n"
		  "<lidaobing@gmail.com>\n"
		  "<mdjhu@sina.com>\n"
		  "<weijian_li88@qq.com>"),
		_("...")
	};
	GtkWidget *frame, *notebook, *sw, *label, *view;
	GtkTextBuffer *buffer;
	gint page;

	more = gtk_dialog_new_with_buttons(_("More about iptux"),
				   NULL, GTK_DIALOG_NO_SEPARATOR,
				   GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
				   NULL);
	gtk_window_set_resizable(GTK_WINDOW(more), FALSE);
	gtk_widget_set_size_request(more, 500, 350);
 
	frame = create_frame(NULL);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(more)->vbox),
				   frame, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 3);

	notebook = gtk_notebook_new();
	gtk_widget_show(notebook);
	gtk_container_add(GTK_CONTAINER(frame), notebook);

	page = 0;
	while (labels[page]) {
		sw = create_scrolled_window();
		label = create_label(labels[page]);
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook), sw, label);
		view = create_text_view();
		gtk_container_add(GTK_CONTAINER(sw), view);

		gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
		gtk_text_view_set_indent(GTK_TEXT_VIEW(view), 20);
		gtk_text_view_set_pixels_inside_wrap(GTK_TEXT_VIEW(view), 5);
		gtk_text_view_set_editable (GTK_TEXT_VIEW(view), FALSE);
	
		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
		gtk_text_buffer_set_text(buffer, text[page], -1);

		page++;
	}
}

void AboutIptux::RunDialog(GtkWidget **dialog)
{
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(*dialog), TRUE);
	g_signal_connect(*dialog, "close",
			  G_CALLBACK(gtk_widget_destroy), NULL);
	g_signal_connect(*dialog, "response",
			  G_CALLBACK(gtk_widget_destroy), NULL);
	g_signal_connect_swapped(*dialog, "destroy",
			  G_CALLBACK(DialogDestroy), dialog);
	gtk_widget_show(*dialog);
}

bool AboutIptux::CheckExist(GtkWidget *dialog)
{
	if (!dialog)
		return false;
	gtk_window_present(GTK_WINDOW(dialog));
	return true;
}

void AboutIptux::DialogDestroy(GtkWidget **dialog)
{
	*dialog = NULL;
}
