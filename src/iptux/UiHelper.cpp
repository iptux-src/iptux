#include "config.h"
#include "UiHelper.h"

#include <atomic>
#include <cerrno>
#include <cstring>
#include <ctime>
#include <glib/gi18n.h>
#include <sys/socket.h>

#include "iptux-core/Models.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"

using namespace std;

namespace iptux {

#if CONFIG_DEBUG
static bool pop_disabled = false;
static atomic_bool open_url_enabled(true);
#else
enum {
  pop_disabled = 0,
  open_url_enabled = 1,
};
#endif

void iptux_open_path(const char* path) {
  g_return_if_fail(!!path);

  GError* error = nullptr;
  gchar* uri = g_filename_to_uri(path, nullptr, &error);
  if (error) {
    LOG_WARN(_("Can't convert path to uri: %s, reason: %s"), path,
             error->message);
    g_error_free(error);
    return;
  }
  if (!open_url_enabled) {
    LOG_INFO("iptux_open_path %s", path);
  } else {
    g_app_info_launch_default_for_uri(uri, nullptr, &error);
  }
  if (error) {
    LOG_WARN(_("Can't open path: %s, reason: %s"), path, error->message);
    g_error_free(error);
  }
  g_free(uri);
}

/**
 * 打开URL.
 * @param url url
 */
void iptux_open_url(const char* url) {
  g_return_if_fail(!!url);

  if (url[0] == '/') {
    iptux_open_path(url);
    return;
  }

  GError* error = nullptr;
  if (!open_url_enabled) {
    LOG_INFO("iptux_open_url %s", url);
  } else {
    g_app_info_launch_default_for_uri(url, nullptr, &error);
  }
  if (error) {
    LOG_WARN(_("Can't open URL: %s, reason: %s"), url, error->message);
    g_error_free(error);
  }
}

bool ValidateDragData(GtkSelectionData* data,
                      GdkDragContext* context,
                      guint time) {
  if (gtk_selection_data_get_length(data) <= 0 ||
      gtk_selection_data_get_format(data) != 8) {
    gtk_drag_finish(context, FALSE, FALSE, time);
    return false;
  }
  return true;
}

void add_accelerator(GtkApplication* app,
                     const char* action,
                     const char* accel) {
  const char* accels[] = {accel, NULL};
  gtk_application_set_accels_for_action(app, action, accels);
}

/**
 * 按照1:1的比例对图片做缩小(请注意，没有放大)处理.
 * @param pixbuf pixbuf
 * @param width width
 * @param height height
 * @note 原pixbuf将被本函数释放
 */
void pixbuf_shrink_scale_1(GdkPixbuf** pixbuf, int width, int height) {
  gdouble scale_x, scale_y, scale;
  gint _width, _height;

  width = (width != -1) ? width : G_MAXINT;
  height = (height != -1) ? height : G_MAXINT;
  _width = gdk_pixbuf_get_width(*pixbuf);
  _height = gdk_pixbuf_get_height(*pixbuf);
  if (_width > width || _height > height) {
    scale = ((scale_x = (gdouble)width / _width) <
             (scale_y = (gdouble)height / _height))
                ? scale_x
                : scale_y;
    _width = (gint)(_width * scale);
    _height = (gint)(_height * scale);
    auto tpixbuf = *pixbuf;
    *pixbuf =
        gdk_pixbuf_scale_simple(tpixbuf, _width, _height, GDK_INTERP_BILINEAR);
    g_object_unref(tpixbuf);
  }
}

/**
 * 让窗体(widget)支持uri拖拽操作.
 * @param widget widget
 */
void widget_enable_dnd_uri(GtkWidget* widget) {
  static const GtkTargetEntry target = {(gchar*)"text/uri-list", 0, 0};

  gtk_drag_dest_set(widget, GTK_DEST_DEFAULT_ALL, &target, 1, GDK_ACTION_MOVE);
}

/**
 * 由(GtkSelectionData)获取(uri)文件链表.
 * @param data selection data
 * @return 文件链表
 */
GSList* selection_data_get_path(GtkSelectionData* data) {
  const char* prl = "file://";
  gchar **uris, **ptr;
  GSList* filelist;

  if (!(uris = gtk_selection_data_get_uris(data)))
    return NULL;

  filelist = NULL;
  ptr = uris;
  while (*ptr) {
    auto uri = g_uri_unescape_string(*ptr, NULL);
    if (strncasecmp(uri, prl, strlen(prl)) == 0)
      filelist = g_slist_append(filelist, g_strdup(uri + strlen(prl)));
    else
      filelist = g_slist_append(filelist, g_strdup(uri));
    g_free(uri);
    ptr++;
  }
  g_strfreev(uris);

  return filelist;
}

#if CONFIG_DEBUG
void pop_disable() {
  pop_disabled = true;
}
void _ForTestToggleOpenUrl(bool enable) {
  open_url_enabled = enable;
}
#endif

/**
 * 弹出消息提示.
 * @param parent parent window
 * @param format as in printf()
 * @param ...
 */
void pop_info(GtkWidget* parent, const gchar* format, ...) {
  GtkWidget* dialog;
  gchar* msg;
  va_list ap;

  va_start(ap, format);
  msg = g_strdup_vprintf(format, ap);
  va_end(ap);

  if (pop_disabled) {
    LOG_INFO("%s\n", msg);
    g_free(msg);
    return;
  }

  dialog = gtk_message_dialog_new(GTK_WINDOW(parent), GTK_DIALOG_MODAL,
                                  GTK_MESSAGE_INFO, GTK_BUTTONS_OK, NULL);
  gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog), msg);
  g_free(msg);
  gtk_window_set_title(GTK_WINDOW(dialog), _("Information"));
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}

/**
 * 弹出警告信息.
 * @param parent parent window
 * @param format as in printf()
 * @param ...
 */
void pop_warning(GtkWidget* parent, const gchar* format, ...) {
  GtkWidget* dialog;
  gchar* msg;
  va_list ap;

  va_start(ap, format);
  msg = g_strdup_vprintf(format, ap);
  va_end(ap);

  if (pop_disabled) {
    LOG_WARN("%s\n", msg);
    g_free(msg);
    return;
  }

  dialog = gtk_message_dialog_new(GTK_WINDOW(parent), GTK_DIALOG_MODAL,
                                  GTK_MESSAGE_INFO, GTK_BUTTONS_OK, NULL);
  gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog), msg);
  g_free(msg);
  gtk_window_set_title(GTK_WINDOW(dialog), _("Warning"));
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}

/**
 * 获取局域网网段名称.
 * @param ipv4 ipv4
 * @return name
 */
string ipv4_get_lan_name(in_addr ipv4) {
  /**
   * @note 局域网网段划分，每两个为一组，以NULL标识结束.
   */
  static const char* localgroup[] = {
      "10.0.0.0",    "10.255.255.255",  "172.16.0.0", "172.31.255.255",
      "192.168.0.0", "192.168.255.255", NULL};
  for (int i = 0; i < 6; i += 2) {
    if (NetSegment(localgroup[i], localgroup[i + 1], "").ContainIP(ipv4)) {
      return stringFormat("%s~%s", localgroup[i], localgroup[i + 1]);
    }
  }
  return "";
}

void g_action_map_set_action_sensitive(GActionMap* map,
                                       const char* action_name,
                                       bool sensitive) {
  GAction* action = g_action_map_lookup_action(G_ACTION_MAP(map), action_name);
  if (!action) {
    return;
  }
  if (!G_IS_SIMPLE_ACTION(action)) {
    return;
  }
  g_simple_action_set_enabled(G_SIMPLE_ACTION(action), sensitive);
}

void g_action_map_set_actions(GActionMap* map,
                              bool sensitive,
                              const char* action_name,
                              va_list args) {
  g_action_map_set_action_sensitive(map, action_name, sensitive);
  const char* action_name2 = va_arg(args, const char*);
  while (action_name2) {
    g_action_map_set_action_sensitive(map, action_name2, sensitive);
    action_name2 = va_arg(args, const char*);
  }
}

void g_action_map_enable_actions(GActionMap* map,
                                 const char* action_name,
                                 ...) {
  va_list args;
  va_start(args, action_name);
  g_action_map_set_actions(map, true, action_name, args);
  va_end(args);
}

void g_action_map_disable_actions(GActionMap* map,
                                  const char* action_name,
                                  ...) {
  va_list args;
  va_start(args, action_name);
  g_action_map_set_actions(map, false, action_name, args);
  va_end(args);
}

string markupEscapeText(const string& str) {
  auto res1 = g_markup_escape_text(str.c_str(), str.size());
  string res(res1);
  g_free(res1);
  return res;
}

const GRegex* getUrlRegex() {
  static GRegex* res = nullptr;
  if (!res) {
    res = g_regex_new(URL_REGEX, GRegexCompileFlags(0), GRegexMatchFlags(0),
                      NULL);
  }
  return res;
}

GActionEntry makeActionEntry(const string& name, GActionCallback f) {
  return GActionEntry(
      {g_strdup(name.c_str()), f, nullptr, nullptr, nullptr, {0, 0, 0}});
}

GActionEntry makeParamActionEntry(const string& name,
                                  GActionCallback f,
                                  const string& paramType) {
  return GActionEntry({g_strdup(name.c_str()),
                       f,
                       g_strdup(paramType.c_str()),
                       nullptr,
                       nullptr,
                       {0, 0, 0}});
}

GActionEntry makeStateActionEntry(const string& name,
                                  GActionCallback f,
                                  const string& paramType,
                                  const string& state) {
  return GActionEntry({g_strdup(name.c_str()),
                       nullptr,
                       g_strdup(paramType.c_str()),
                       g_strdup(state.c_str()),
                       f,
                       {0, 0, 0}});
}

gboolean gtk_window_iconify_on_delete(GtkWindow* window) {
  gtk_window_iconify(window);
  return TRUE;
}

GtkHeaderBar* CreateHeaderBar(GtkWindow* window, GMenuModel* menu) {
  GtkBuilder* builder =
      gtk_builder_new_from_resource(IPTUX_RESOURCE "gtk/HeaderBar.ui");
  GtkHeaderBar* headerBar =
      GTK_HEADER_BAR(gtk_builder_get_object(builder, "header_bar"));
  gtk_header_bar_set_has_subtitle(headerBar, FALSE);
  auto menuButton = gtk_builder_get_object(builder, "menu_button");
  gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(menuButton), menu);
  gtk_window_set_titlebar(window, GTK_WIDGET(headerBar));

  g_object_unref(builder);
  return headerBar;
}

std::string TimeToStr(time_t t) {
  time_t now = time(nullptr);
  return TimeToStr_(t, now);
}

std::string TimeToStr_(time_t t, time_t now) {
  struct tm tm_t;
  struct tm tm_now;
  localtime_r(&t, &tm_t);
  localtime_r(&now, &tm_now);
  char res[11];
  if (tm_t.tm_year == tm_now.tm_year && tm_t.tm_yday == tm_now.tm_yday) {
    strftime(res, sizeof(res), "%H:%M", &tm_t);
  } else {
    strftime(res, sizeof(res), "%F", &tm_t);
  }
  return res;
}

string StrFirstNonEmptyLine(const string& s) {
  size_t pos = s.find_first_not_of(" \r\n");
  if (pos == string::npos) {
    return "";
  }
  size_t pos2 = s.find_first_of("\r\n", pos);
  if (pos2 == string::npos) {
    return s.substr(pos);
  }
  return s.substr(pos, pos2 - pos);
}

GtkImage* igtk_image_new_with_size(const char* filename,
                                   int width,
                                   int height) {
  GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
  if (!pixbuf) {
    LOG_ERROR("Error loading image.");
    return NULL;
  }

  pixbuf_shrink_scale_1(&pixbuf, width, height);
  return GTK_IMAGE(gtk_image_new_from_pixbuf(pixbuf));
}

string igtk_text_buffer_get_text(GtkTextBuffer* buffer) {
  GtkTextIter start, end;
  gtk_text_buffer_get_bounds(buffer, &start, &end);
  char* res1 = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
  string res(res1);
  g_free(res1);
  return res;
}

}  // namespace iptux
