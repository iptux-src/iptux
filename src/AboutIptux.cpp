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

	more = gtk_dialog_new_with_buttons(_("More information about iptux"),
					   NULL, GTK_DIALOG_NO_SEPARATOR,
					   GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
					   NULL);
	gtk_window_set_resizable(GTK_WINDOW(more), FALSE);
	gtk_widget_set_size_request(more, 400, 300);

	frame = create_frame(NULL);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(more)->vbox), frame,
							   TRUE, TRUE, 0);
	label = create_label("liangsuilong@gmail.com\n"
			"lidaobing@gmail.com\n"
			"mdjhu@sina.com");
	gtk_container_add(GTK_CONTAINER(frame), label);
	/*以上代码内容无效，请自行编码*/
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
