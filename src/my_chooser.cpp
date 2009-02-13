//
// C++ Implementation: my_chooser
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "my_chooser.h"
#include "support.h"
#include "udt.h"

my_chooser::my_chooser(const gchar * t, GtkWidget * p):title(t), parent(p),
chooser(NULL)
{
}

my_chooser::~my_chooser()
{
}

gchar *my_chooser::choose_file(const gchar * t, GtkWidget * p)
{
	my_chooser mc(t, p);

	mc.create_chooser();
	return mc.run_chooser();
}

void my_chooser::create_chooser()
{
	GtkWidget *preview;

	chooser = gtk_file_chooser_dialog_new(title, GTK_WINDOW(parent),
					      GTK_FILE_CHOOSER_ACTION_OPEN,
					      GTK_STOCK_CANCEL,
					      GTK_RESPONSE_CANCEL,
					      GTK_STOCK_OPEN,
					      GTK_RESPONSE_ACCEPT, NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(chooser),
					GTK_RESPONSE_ACCEPT);

	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(chooser), TRUE);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(chooser), FALSE);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER
						       (chooser), TRUE);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser),
					    g_get_home_dir());

	preview = gtk_image_new();
	gtk_widget_set_size_request(preview, MAX_PREVIEWSIZE, MAX_PREVIEWSIZE);
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(chooser), preview);
	gtk_file_chooser_set_preview_widget_active(GTK_FILE_CHOOSER(chooser),
						   FALSE);
	g_signal_connect(chooser, "update-preview",
			  G_CALLBACK(UpdatePreview), preview);
}

gchar *my_chooser::run_chooser()
{
	int result;
	gchar *filename;

	result = gtk_dialog_run(GTK_DIALOG(chooser));
	if (result == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(
			GTK_FILE_CHOOSER(chooser));
	else
		filename = NULL;
	gtk_widget_destroy(chooser);

	return filename;
}

void my_chooser::UpdatePreview(GtkFileChooser * chooser, GtkWidget * preview)
{
	gchar *filename;
	GdkPixbuf *pixbuf;

	filename = gtk_file_chooser_get_preview_filename(chooser);
	if (!filename) {
		gtk_file_chooser_set_preview_widget_active(chooser, FALSE);
		return;
	}

	pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
	g_free(filename);
	if (!pixbuf) {
		gtk_file_chooser_set_preview_widget_active(chooser, FALSE);
		return;
	}

	pixbuf_shrink_scale_1(&pixbuf, MAX_PREVIEWSIZE, MAX_PREVIEWSIZE);
	gtk_image_set_from_pixbuf(GTK_IMAGE(preview), pixbuf);
	g_object_unref(pixbuf);
	gtk_file_chooser_set_preview_widget_active(chooser, TRUE);
}
