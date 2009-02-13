//
// C++ Implementation: output
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "output.h"

void pwarning(enum STATE_TYPE state, const char *format, ...)
{
	va_list ap;

#ifdef WARNING
	va_start(ap, format);
	vwarnx(format, ap);
	va_end(ap);
#endif
	switch (state) {
	case Quit:
		exit(1);
		break;
	default:
		break;
	}
}

void pmessage(const char *format, ...)
{
	va_list ap;

#ifdef MESSAGE
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
#endif
}

void ptrace(const char *format, ...)
{
	va_list ap;

#ifdef TRACE
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
#endif
}

void pop_info(GtkWidget * parent, GtkWidget * fw, const gchar * format, ...)
{
	GtkWidget *dialog;
	va_list ap;
	gchar *msg;

	va_start(ap, format);
	msg = g_strdup_vprintf(format, ap);
	va_end(ap);
	dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(parent),
						    GTK_DIALOG_MODAL,
						    GTK_MESSAGE_INFO,
						    GTK_BUTTONS_OK, msg);
	g_free(msg);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Infomation"));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	if (fw)
		gtk_widget_grab_focus(fw);
}

void pop_warning(GtkWidget * parent, GtkWidget * fw, const gchar * format, ...)
{
	GtkWidget *dialog;
	va_list ap;
	gchar *msg;

	va_start(ap, format);
	msg = g_strdup_vprintf(format, ap);
	va_end(ap);
	dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(parent),
						    GTK_DIALOG_MODAL,
						    GTK_MESSAGE_WARNING,
						    GTK_BUTTONS_OK, msg);
	g_free(msg);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Warning"));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	if (fw)
		gtk_widget_grab_focus(fw);
}

void pop_error(const gchar * format, ...)
{
	GtkWidget *dialog;
	va_list ap;
	gchar *msg;

	va_start(ap, format);
	msg = g_strdup_vprintf(format, ap);
	va_end(ap);
	dialog = gtk_message_dialog_new_with_markup(NULL,
						    GTK_DIALOG_MODAL,
						    GTK_MESSAGE_ERROR,
						    GTK_BUTTONS_OK, msg);
	g_free(msg);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}
