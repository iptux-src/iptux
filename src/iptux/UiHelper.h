#ifndef IPTUX_UIHELPER_H
#define IPTUX_UIHELPER_H

#include <gtk/gtk.h>

namespace iptux {

void iptux_init();

bool ValidateDragData(GtkSelectionData *data, GdkDragContext *context,
                      guint time);

void add_accelerator(GtkApplication* app, const char* action, const char* accel);
void pixbuf_shrink_scale_1(GdkPixbuf **pixbuf, int width, int height);

void widget_enable_dnd_uri(GtkWidget *widget);
GSList *selection_data_get_path(GtkSelectionData *data);
void pop_info(GtkWidget *parent, const gchar *format, ...);
void pop_warning(GtkWidget *parent, const gchar *format, ...);
void pop_error(const gchar *format, ...);
void iptux_open_url(const char *url);

}
#endif //IPTUX_UIHELPER_H
