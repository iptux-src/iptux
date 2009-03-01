//
// C++ Implementation: my_entry
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "my_entry.h"
#include "baling.h"

my_entry::my_entry()
{
}

my_entry::~my_entry()
{
}

GtkWidget *my_entry::create_entry(const char *text, const char *tip,
				  bool digital)
{
	GtkWidget *entry;

	entry = gtk_entry_new();
	gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);
	if (text)
		gtk_entry_set_text(GTK_ENTRY(entry), text);
	if (tip) {
		g_object_set(entry, "has-tooltip", TRUE, NULL);
		g_signal_connect(entry, "query-tooltip",
				 G_CALLBACK(QueryTooltip), (gpointer) tip);
	}
	if (digital)
		g_signal_connect(entry, "insert-text",
				 G_CALLBACK(InsertText), NULL);
	gtk_widget_show(entry);

	return entry;
}

gboolean my_entry::QueryTooltip(GtkWidget * widget, gint x, gint y,
				gboolean key, GtkTooltip * tooltip,
				gpointer data)
{
	GtkWidget *label;

	label = create_label((char *)data);
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_label_set_line_wrap_mode(GTK_LABEL(label), PANGO_WRAP_WORD_CHAR);
	gtk_tooltip_set_custom(tooltip, label);

	return TRUE;
}

void my_entry::InsertText(GtkEditable * editable, gchar * text, gint length,
			  gint * position, gpointer data)
{
	gint count;

	if (length == -1)
		length = strlen(text);
	count = 0;
	while (count < length) {
		if (!isdigit(*(text + count)) && !(*(text + count) == '.')) {
			g_signal_stop_emission_by_name(editable, "insert-text");
			return;
		}
		count++;
	}
}
