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
#include "my_entry.h"
#include "output.h"
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
				    GTK_WINDOW(inter.window), GTK_DIALOG_MODAL,
				    _("Refuse"), GTK_RESPONSE_CANCEL,
				    _("Agree"), GTK_RESPONSE_ACCEPT, NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog),
					GTK_RESPONSE_ACCEPT);

	box = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), box,
						   TRUE, TRUE, 0);

	image = gtk_image_new_from_stock(GTK_STOCK_DIALOG_QUESTION,
						 GTK_ICON_SIZE_DIALOG);
	gtk_widget_show(image);
	gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE, 0);
	image = gtk_vseparator_new();
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

char *pop_obtain_passwd()
{
	extern struct interactive inter;
	GtkWidget *dialog, *frame, *box, *vbox;
	GtkWidget *image, *passwd;
	gchar *text;
	gint result;

	gdk_threads_enter();
	dialog = gtk_dialog_new_with_buttons(_("Access password"),
		    GTK_WINDOW(inter.window), GTK_DIALOG_MODAL,
		    GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	frame = create_frame(_("Please enter the password for "
					"access to shared files"));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), frame,
						   FALSE, FALSE, 0);
	box = create_box(FALSE);
	gtk_container_add(GTK_CONTAINER(frame), box);

	image = gtk_image_new_from_stock(GTK_STOCK_DIALOG_AUTHENTICATION,
							GTK_ICON_SIZE_DIALOG);
	gtk_widget_show(image);
	gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE, 0);
	image = gtk_vseparator_new();
	gtk_widget_show(image);
	gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE, 0);
	vbox = create_box();
	gtk_box_pack_start(GTK_BOX(box), vbox, TRUE, TRUE, 4);
	box = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), box, FALSE, FALSE, 0);
	passwd = create_label(_("Password: "));
	gtk_box_pack_start(GTK_BOX(box), passwd, FALSE, FALSE, 0);
	passwd = my_entry::create_entry(NULL, NULL);
	gtk_entry_set_visibility(GTK_ENTRY(passwd), FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), passwd, TRUE, TRUE, 0);

mark:	if ((result = gtk_dialog_run(GTK_DIALOG(dialog))) == GTK_RESPONSE_OK) {
		text = gtk_editable_get_chars(GTK_EDITABLE(passwd), 0, -1);
		if (*text == '\0') {
			g_free(text);
			pop_warning(dialog, passwd, _("\nPassword is blank!"));
			goto mark;
		}
	}
	gtk_widget_destroy(dialog);
	gdk_threads_leave();

	return (result == GTK_RESPONSE_OK) ? text : NULL;
}

char *pop_passwd_setting(GtkWidget *parent)
{
	GtkWidget *dialog, *hbox, *passwd, *repeat;
	gchar *text1, *text2;
	gint result;

	dialog = gtk_dialog_new_with_buttons(_("Please enter a new password"),
				    GTK_WINDOW(parent), GTK_DIALOG_MODAL,
				    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				    GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox,
						   FALSE, FALSE, 0);
	passwd = create_label(_("Password: "));
	gtk_box_pack_start(GTK_BOX(hbox), passwd, FALSE, FALSE, 0);
	passwd = my_entry::create_entry(NULL, NULL);
	gtk_entry_set_visibility(GTK_ENTRY(passwd), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), passwd, TRUE, TRUE, 0);
	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox,
						   FALSE, FALSE, 0);
	repeat = create_label(_("Repeat: "));
	gtk_box_pack_start(GTK_BOX(hbox), repeat, FALSE, FALSE, 0);
	repeat = my_entry::create_entry(NULL, NULL);
	gtk_entry_set_visibility(GTK_ENTRY(repeat), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), repeat, TRUE, TRUE, 0);

mark:	if ((result = gtk_dialog_run(GTK_DIALOG(dialog))) == GTK_RESPONSE_OK) {
		text1 = gtk_editable_get_chars(GTK_EDITABLE(passwd), 0, -1);
		text2 = gtk_editable_get_chars(GTK_EDITABLE(repeat), 0, -1);
		if (strcmp(text1, text2) != 0) {
			g_free(text1), g_free(text2);
			pop_warning(dialog, passwd,
				    _("\nEnter the password twice inconsistent!"));
			goto mark;
		} else if (*text1 == '\0') {
			g_free(text1), g_free(text2);
			pop_warning(dialog, passwd, _("\nPassword is blank!"));
			goto mark;
		}
	}
	gtk_widget_destroy(dialog);

	if (result == GTK_RESPONSE_OK) {
		g_free(text1);
		return text2;
	}
	return NULL;
}
