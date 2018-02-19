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
void bind_iptux_port(int port) {
  struct sockaddr_in addr;
  int tcpsock, udpsock;

  tcpsock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_enable_reuse(tcpsock);
  udpsock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  socket_enable_reuse(udpsock);
  socket_enable_broadcast(udpsock);
  if ((tcpsock == -1) || (udpsock == -1)) {
    int ec = errno;
    const char* errmsg = g_strdup_printf(_("Fatal Error!! Failed to create new socket!\n%s"),
                                         strerror(ec));
    LOG_WARN("%s", errmsg);
    throw BindFailedException(ec, errmsg);
  }

  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if (::bind(tcpsock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    int ec = errno;
    close(tcpsock);
    close(udpsock);
    const char* errmsg = g_strdup_printf(_("Fatal Error!! Failed to bind the TCP port(%d)!\n%s"),
                                         port, strerror(ec));
    LOG_WARN("%s", errmsg);
    throw BindFailedException(ec, errmsg);
  }
  if(::bind(udpsock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    int ec = errno;
    close(tcpsock);
    close(udpsock);
    const char* errmsg = g_strdup_printf(_("Fatal Error!! Failed to bind the UDP port(%d)!\n%s"),
                                         port, strerror(ec));
    LOG_WARN("%s", errmsg);
    throw BindFailedException(ec, errmsg);
  }
  g_cthrd->setTcpSock(tcpsock);
  g_cthrd->setUdpSock(udpsock);
}

/**
 * 程序必要初始化.
 */
void iptux_init(int port) {
  bind_iptux_port(port);
  init_iptux_environment();

  g_progdt->InitSublayer();
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
