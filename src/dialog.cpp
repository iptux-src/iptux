//
// C++ Implementation: dialog
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "dialog.h"
#include "baling.h"
#include "Pal.h"

bool pop_request_quit()
{
	extern struct interactive inter;
	GtkWidget *dialog;
	gint result;

	dialog = gtk_message_dialog_new(GTK_WINDOW(inter.window),
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_QUESTION,
					GTK_BUTTONS_OK_CANCEL,
					_("The file transfer is running!"
					  "\nAre you sure you want to quit?"));
	gtk_window_set_title(GTK_WINDOW(dialog), _("Confirm close"));

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	if (result == GTK_RESPONSE_OK)
		return true;
	return false;
}

bool pop_request_shared(gpointer data)
{
	extern struct interactive inter;
	GtkWidget *dialog, *box;
	GtkWidget *label, *image;
	char ipstr[INET_ADDRSTRLEN], *ptr;
	bool result;

	gdk_threads_enter();
	dialog = gtk_dialog_new_with_buttons(_("Request for shared resources"),
					     GTK_WINDOW(inter.window),
					     GTK_DIALOG_MODAL, _("Refuse"),
					     GTK_RESPONSE_CANCEL, _("Agree"),
					     GTK_RESPONSE_ACCEPT, NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog),
					GTK_RESPONSE_ACCEPT);

	box = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), box, TRUE, TRUE, 0);

	image = gtk_image_new_from_stock(GTK_STOCK_DIALOG_QUESTION,
						 GTK_ICON_SIZE_DIALOG);
	gtk_widget_show(image);
	gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE, 0);

	inet_ntop(AF_INET, &((Pal *) data)->Ipv4Quote(), ipstr,
					  INET_ADDRSTRLEN);
	ptr = g_strdup_printf(_("The pal (%s)[%s]\nis requesting for "
				"your shared resources,\nagree or not?"),
			        ((Pal *) data)->NameQuote(), ipstr);
	label = create_label(ptr);
	free(ptr);
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_label_set_line_wrap_mode(GTK_LABEL(label), PANGO_WRAP_WORD_CHAR);
	gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 4);

	result = (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT);
	gtk_widget_destroy(dialog);
	gdk_threads_leave();

	return result;
}
