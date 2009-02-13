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
AboutIptux::AboutIptux()
{
}

AboutIptux::~AboutIptux()
{
}

void AboutIptux::AboutEntry()
{
	AboutIptux ai;

	if (AboutIptux::CheckExist())
		return;
	ai.CreateAbout();
	ai.RunAbout();
}

void AboutIptux::CreateAbout()
{
	const char *authors[] = {
		"Jally\tjallyx@163.com",
		NULL
	};
	const char *artists[] = {
		"Jally\tjallyx@163.com",
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
				      _("Lan communication software"));
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about),
				     _("Home: http://code.google.com/p/iptux/"));
	label = create_label(_("Forum: https://groups.google.com/group/iptux/"));
	gtk_box_pack_end(GTK_BOX(GTK_DIALOG(about)->vbox), label,
							    FALSE, FALSE, 0);
	gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(about), "GPL 2+");
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about), authors);
	gtk_about_dialog_set_artists(GTK_ABOUT_DIALOG(about), artists);
	gtk_about_dialog_set_translator_credits(GTK_ABOUT_DIALOG(about),
								translators);
	pixbuf = gdk_pixbuf_new_from_file(__LOGO_DIR "/tux.png", NULL);
	if (pixbuf) {
		gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(about), pixbuf);
		g_object_unref(pixbuf);
	}
}

void AboutIptux::RunAbout()
{
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(about), TRUE);
	g_signal_connect(about, "response", G_CALLBACK(gtk_widget_destroy), NULL);
	g_signal_connect(about, "destroy", G_CALLBACK(AboutDestroy), NULL);
	gtk_widget_show(about);
}

bool AboutIptux::CheckExist()
{
	if (!about)
		return false;
	gtk_window_present(GTK_WINDOW(about));
	return true;
}

void AboutIptux::AboutDestroy()
{
	about = NULL;
}
