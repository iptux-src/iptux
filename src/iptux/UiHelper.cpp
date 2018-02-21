#include "config.h"
#include "UiHelper.h"

#include <cstring>
#include <cerrno>

#include <unistd.h>
#include <sys/socket.h>

#include <glib/gi18n.h>

#include "iptux/output.h"
#include "iptux/support.h"
#include "iptux/global.h"

using namespace std;

namespace iptux {

/**
 * 绑定iptux程序的服务监听端口.
 */

/**
 * 程序必要初始化.
 */
void iptux_init() {
  init_iptux_environment();

  g_lgsys->InitSublayer();
  g_sndsys->InitSublayer();

  signal(SIGPIPE, SIG_IGN);
  g_lgsys->SystemLog(_("Loading the process successfully!"));
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

}
