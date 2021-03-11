#include "config.h"
#include "UiHelper.h"

#include <cstring>
#include <cerrno>

#include <unistd.h>
#include <sys/socket.h>

#include <glib/gi18n.h>

#include "iptux-core/Models.h"
#include "iptux-utils/utils.h"
#include "iptux-utils/output.h"

using namespace std;

namespace iptux {

/**
 * 打开URL.
 * @param url url
 */
void iptux_open_url(const char *url) {
  int fd;

  if (fork() != 0) return;

  /* 关闭由iptux打开的所有可能的文件描述符 */
  fd = 3;
  while (fd < FD_SETSIZE) {
    close(fd);
    fd++;
  }
  /* 脱离终端控制 */
  setsid();

  /* 打开URL */
  execlp("xdg-open", "xdg-open", url, NULL);
  /* 测试系统中所有可能被安装的浏览器 */
  execlp("firefox", "firefox", url, NULL);
  execlp("opera", "opera", url, NULL);
  execlp("konqueror", "konqueror", url, NULL);
  execlp("open", "open", url, NULL);
  LOG_WARN(_("Can't find any available web browser!\n"));
}

bool ValidateDragData(GtkSelectionData *data, GdkDragContext *context,
                      guint time) {
  if (gtk_selection_data_get_length(data) <= 0 ||
      gtk_selection_data_get_format(data) != 8) {
    gtk_drag_finish(context, FALSE, FALSE, time);
    return false;
  }
  return true;
}

void add_accelerator(GtkApplication *app, const char *action, const char *accel) {
  const char *accels[] = {
      accel,
      NULL
  };
  gtk_application_set_accels_for_action(app, action, accels);
}

/**
 * 按照1:1的比例对图片做缩小(请注意，没有放大)处理.
 * @param pixbuf pixbuf
 * @param width width
 * @param height height
 * @note 原pixbuf将被本函数释放
 */
void pixbuf_shrink_scale_1(GdkPixbuf **pixbuf, int width, int height) {
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
void widget_enable_dnd_uri(GtkWidget *widget) {
  static const GtkTargetEntry target = {(gchar *)"text/uri-list", 0, 0};

  gtk_drag_dest_set(widget, GTK_DEST_DEFAULT_ALL, &target, 1, GDK_ACTION_MOVE);
}

/**
 * 由(GtkSelectionData)获取(uri)文件链表.
 * @param data selection data
 * @return 文件链表
 */
GSList *selection_data_get_path(GtkSelectionData *data) {
  const char *prl = "file://";
  gchar **uris, **ptr;
  GSList *filelist;

  if (!(uris = gtk_selection_data_get_uris(data))) return NULL;

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

/**
 * 弹出消息提示.
 * @param parent parent window
 * @param format as in printf()
 * @param ...
 */
void pop_info(GtkWidget *parent, const gchar *format, ...) {
  GtkWidget *dialog;
  gchar *msg;
  va_list ap;

  va_start(ap, format);
  msg = g_strdup_vprintf(format, ap);
  va_end(ap);
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
void pop_warning(GtkWidget *parent, const gchar *format, ...) {
  GtkWidget *dialog;
  gchar *msg;
  va_list ap;

  va_start(ap, format);
  msg = g_strdup_vprintf(format, ap);
  va_end(ap);
  dialog = gtk_message_dialog_new(GTK_WINDOW(parent), GTK_DIALOG_MODAL,
                                  GTK_MESSAGE_INFO, GTK_BUTTONS_OK, NULL);
  gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog), msg);
  g_free(msg);
  gtk_window_set_title(GTK_WINDOW(dialog), _("Warning"));
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}

/**
 * 严重错误，程序将有可能自行强制退出.
 * @param format as in printf()
 * @param ...
 */
void pop_error(const gchar *format, ...) {
  GtkWidget *dialog;
  gchar *msg;
  va_list ap;

  va_start(ap, format);
  msg = g_strdup_vprintf(format, ap);
  va_end(ap);
  dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
                                  GTK_BUTTONS_OK, NULL);
  gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog), msg);
  g_free(msg);
  gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
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
  static const char *localgroup[] = {
      "10.0.0.0",    "10.255.255.255",  "172.16.0.0", "172.31.255.255",
      "192.168.0.0", "192.168.255.255", NULL};
  for(int i = 0; i < 6; i+=2) {
    if(NetSegment(localgroup[i], localgroup[i+1], "").ContainIP(ipv4)) {
      return stringFormat("%s~%s", localgroup[i], localgroup[i+1]);
    }
  }
  return "";
}

}
