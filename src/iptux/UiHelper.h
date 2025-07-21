#ifndef IPTUX_UIHELPER_H
#define IPTUX_UIHELPER_H

#include <string>

#include <arpa/inet.h>
#include <gtk/gtk.h>

namespace iptux {

typedef void (*GActionCallback)(GSimpleAction* action,
                                GVariant* parameter,
                                gpointer user_data);
#define G_ACTION_CALLBACK(f) ((GActionCallback)(f))

bool ValidateDragData(GtkSelectionData* data,
                      GdkDragContext* context,
                      guint time);

const GRegex* getUrlRegex();

gboolean gtk_window_iconify_on_delete(GtkWindow* window);
void add_accelerator(GtkApplication* app,
                     const char* action,
                     const char* accel);
void pixbuf_shrink_scale_1(GdkPixbuf** pixbuf, int width, int height);

void widget_enable_dnd_uri(GtkWidget* widget);
GSList* selection_data_get_path(GtkSelectionData* data);

/**
 * @brief create GtkImage with width and height
 *
 * @param filename image file name
 * @param width max width, -1 for no limit
 * @param height max height, -1 for no limit
 * @return GtkImage* null if failed
 */
GtkImage* igtk_image_new_with_size(const char* filename, int width, int height);
std::string igtk_text_buffer_get_text(GtkTextBuffer* buffer);

/**
 * @brief only used for test, after call this, pop_info, pop_warning,
 * and iptux_open_url will only print log
 */
void pop_disable();
void pop_info(GtkWidget* parent, const gchar* format, ...) G_GNUC_PRINTF(2, 3);
void pop_warning(GtkWidget* parent, const gchar* format, ...)
    G_GNUC_PRINTF(2, 3);
void iptux_open_url(const char* url);
void _ForTestToggleOpenUrl(bool enable);

std::string ipv4_get_lan_name(in_addr ipv4);

void g_action_map_enable_actions(GActionMap* map,
                                 const char* action_name,
                                 ...) G_GNUC_NULL_TERMINATED;
void g_action_map_disable_actions(GActionMap* map,
                                  const char* action_name,
                                  ...) G_GNUC_NULL_TERMINATED;

GActionEntry makeActionEntry(const char* name, GActionCallback f);
GActionEntry makeParamActionEntry(const char* name,
                                  GActionCallback f,
                                  const char* paramType);
GActionEntry makeStateActionEntry(const char* name,
                                  GActionCallback f,
                                  const char* paramType,
                                  const char* state);

std::string StrFirstNonEmptyLine(const std::string& s);

/**
 * @brief wrapper for g_makeup_escape_text
 *
 * @param str
 * @return std::string
 */
std::string markupEscapeText(const std::string& str);

template <typename... Args>
std::string MarkupPrintf(const char* format, ...) G_GNUC_PRINTF(1, 2);

template <typename... Args>
std::string MarkupPrintf(const char* format, ...) {
  va_list args;

  va_start(args, format);
  gchar* buf = g_markup_vprintf_escaped(format, args);
  va_end(args);
  std::string res(buf, strlen(buf));
  g_free(buf);
  return res;
}

/**
 * @brief create a headerbar with menu, and set this headerbar to the window
 *
 * we need to set to window inside this func, otherwise we need manage the
 * refcount manually.
 *
 */
GtkHeaderBar* CreateHeaderBar(GtkWindow* window, GMenuModel* menu);

std::string TimeToStr(time_t t);

/* only used for test */
std::string TimeToStr_(time_t t, time_t now);

}  // namespace iptux
#endif  // IPTUX_UIHELPER_H
