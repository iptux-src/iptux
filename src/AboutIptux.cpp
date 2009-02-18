//
// C++ Implementation: AboutIptux
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
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
void 
AboutIptux::activate_url (GtkAboutDialog *about,
	      const gchar    *link,
	      gpointer        data)
	

{
  g_print ("show url %s\n", link);
}



void AboutIptux::CreateAbout()
{
	const char *authors[] = {
		"Jally\tjallyx@163.com",
		NULL
	};
	const char *artists[] = {
		"\t\tweijian_li88＠qq.com",
		"ManPT\tpentie@gmail.com",
		"Jally\t\tjallyx@163.com",
		NULL
	};
	const char *translators = _("LiJinhui\nLiuTao");
	GtkWidget *label;
	GdkPixbuf *pixbuf;

	about = gtk_about_dialog_new();
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about), _("iptux"));
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about), "0.4.5");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about),
				       "Copyright © 2008-2009 by Jally");
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about),
				      _("A GTK+ based LAN Messenger."));
    gtk_about_dialog_set_url_hook (activate_url, NULL, NULL);
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about),
				     _("http://code.google.com/p/iptux/"));
    /*
	label = create_label(_("Forum: https://groups.google.com/group/iptux/"));

	gtk_box_pack_end(GTK_BOX(GTK_DIALOG(about)->vbox), label,
							    FALSE, FALSE, 0);
    */
	gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(about), "GPL 2+");
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about), authors);
	gtk_about_dialog_set_artists(GTK_ABOUT_DIALOG(about), artists);
	gtk_about_dialog_set_translator_credits(GTK_ABOUT_DIALOG(about),
								translators);
	pixbuf = gdk_pixbuf_new_from_file(__LOGO_DIR "/ip-tux.png", NULL);
	if (pixbuf) {
		gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(about), pixbuf);
		g_object_unref(pixbuf);
	}
}


/**
 * 请添加更多内容，诸如相关帮助文档的URL，程序的热心帮助者等
 * 具体说来，应该出现但是关于对话框中却没有写出来的内容，都应该会出现在此处
 * 为什么要这样做？原因很简单，iptux并不仅仅是程序开发组的劳动成果
 */
void AboutIptux::CreateMore()
{
	GtkWidget *frame, *label;
    char *labels[] = {_("Help"), _("Contributers"), _("...")};
    char *text[] = {
        _("Project Home: \nhttp://code.google.com/p/iptux/\n\n"
        "User and Developer Group: \nhttps://groups.google.com/group/iptux\n\n"
        "Note that you can get help form the project wiki page.\n\n"
        "If you find no solutions in any of the existed documents, feel free to drop a email at iptux@googlegroups.com, lots of users and developers would be glade to help about your problems."),
        _("It's an honer that IPTUX was contributed voluntarilly by many people. Without your help, IPTUX could never make it.\n\n"
        "Especially, Here's some we would like to thank much:\n\n"
        "<liangsuilong@gmail.com>\n"
		"<lidaobing@gmail.com>\n"
		"<mdjhu@sina.com>"
        ),
        _("...")};
    
	more = gtk_dialog_new_with_buttons(_("More about IPTUX"),
					   NULL, GTK_DIALOG_NO_SEPARATOR,
					   GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
					   NULL);
	gtk_window_set_resizable(GTK_WINDOW(more), FALSE);
	gtk_widget_set_size_request(more, 400, 300);

	frame = gtk_frame_new (NULL);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(more)->vbox), frame,
							   TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    GtkWidget *notebook;
    notebook = gtk_notebook_new ();
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);

    for(int page = 0; page < 3; page ++){

        GtkWidget *scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy (
                    GTK_SCROLLED_WINDOW (scrolledwindow), 
                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

        GtkWidget *textview;
        textview = gtk_text_view_new ();
        gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview),
                    GTK_WRAP_WORD);
        gtk_text_view_set_indent (GTK_TEXT_VIEW (textview), 20);
        gtk_text_view_set_pixels_inside_wrap (GTK_TEXT_VIEW (textview), 5);
    //    gtk_container_set_border_width (GTK_CONTAINER (textview), 5);
        gtk_text_view_set_editable (GTK_TEXT_VIEW (textview), FALSE);
        gtk_container_add (GTK_CONTAINER (scrolledwindow), textview);

        GtkTextBuffer *buffer;
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
        show_help_page(buffer, text[page]);

        label = gtk_label_new (labels[page]);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                    scrolledwindow, label);
    }
    gtk_container_add(GTK_CONTAINER(frame), notebook);
    gtk_widget_show_all(frame);
}


void AboutIptux::show_help_page(GtkTextBuffer *buffer, const char * str)
{
    GtkTextIter iter;
    gtk_text_buffer_set_text (buffer, "", 0);
    gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);

    gtk_text_buffer_insert (buffer, &iter, str, -1);
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
