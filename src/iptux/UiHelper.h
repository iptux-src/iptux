#ifndef IPTUX_UIHELPER_H
#define IPTUX_UIHELPER_H

#include <string>

#include <arpa/inet.h>
#include <gtk/gtk.h>

namespace iptux {

typedef void (* GActionCallback) (GSimpleAction *action,
                                  GVariant      *parameter,
                                  gpointer       user_data) ;
#define	G_ACTION_CALLBACK(f)			 ((GActionCallback) (f))

bool ValidateDragData(GtkSelectionData *data, GdkDragContext *context,
                      guint time);

void add_accelerator(GtkApplication* app, const char* action, const char* accel);
void pixbuf_shrink_scale_1(GdkPixbuf **pixbuf, int width, int height);

void widget_enable_dnd_uri(GtkWidget *widget);
GSList *selection_data_get_path(GtkSelectionData *data);
void pop_info(GtkWidget *parent, const gchar *format, ...) G_GNUC_PRINTF (2, 3);
void pop_warning(GtkWidget *parent, const gchar *format, ...) G_GNUC_PRINTF (2, 3);
void iptux_open_url(const char *url);
std::string ipv4_get_lan_name(in_addr ipv4);

}
#endif //IPTUX_UIHELPER_H
