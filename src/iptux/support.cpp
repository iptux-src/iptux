//
// C++ Implementation: support
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "support.h"

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include "CoreThread.h"
#include "LogSystem.h"
#include "MainWindow.h"
#include "ProgramData.h"
#include "SoundSystem.h"
#include "dialog.h"
#include "global.h"
#include "ipmsg.h"
#include "iptux/config.h"
#include "iptux/deplib.h"
#include "output.h"
#include "utils.h"

namespace iptux {

static void bind_iptux_port(int port);

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
  signal(SIGHUP, iptux_quit);
  signal(SIGINT, iptux_quit);
  signal(SIGQUIT, iptux_quit);
  signal(SIGTERM, iptux_quit);

  g_lgsys->SystemLog(_("Loading the process successfully!"));
}

/**
 * 程序GUI退出.
 */
void iptux_gui_quit() {
  if (g_mwin->TransmissionActive() && !pop_request_quit()) return;
  gtk_main_quit();
  iptux_quit(0);
}

/**
 * 程序底层退出.
 */
void iptux_quit(int _ignore) {
  g_lgsys->SystemLog(_("The process is about to quit!"));
  exit(0);
}

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
  pwarning(_("Can't find any available web browser!\n"));
}

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
  if (bind(tcpsock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    int ec = errno;
    close(tcpsock);
    close(udpsock);
    const char* errmsg = g_strdup_printf(_("Fatal Error!! Failed to bind the TCP port(%d)!\n%s"),
                                         port, strerror(ec));
    LOG_WARN("%s", errmsg);
    throw BindFailedException(ec, errmsg);
  }
  if(bind(udpsock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
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
 * 初始化程序iptux的运行环境.
 * cache iptux {pic, photo, icon} \n
 * config iptux {log, photo, icon} \n
 */
void init_iptux_environment() {
  const char *env;
  char path[MAX_PATHLEN];

  env = g_get_user_cache_dir();
  if (access(env, F_OK) != 0) mkdir(env, 0777);
  snprintf(path, MAX_PATHLEN, "%s" IPTUX_PATH, env);
  if (access(path, F_OK) != 0) mkdir(path, 0777);
  snprintf(path, MAX_PATHLEN, "%s" PIC_PATH, env);
  if (access(path, F_OK) != 0) mkdir(path, 0777);
  snprintf(path, MAX_PATHLEN, "%s" PHOTO_PATH, env);
  if (access(path, F_OK) != 0) mkdir(path, 0777);
  snprintf(path, MAX_PATHLEN, "%s" ICON_PATH, env);
  if (access(path, F_OK) != 0) mkdir(path, 0777);
  snprintf(path, MAX_PATHLEN, "%s" LOG_PATH, env);
  if (access(path, F_OK) != 0) mkdir(path, 0777);

  env = g_get_user_config_dir();
  if (access(env, F_OK) != 0) mkdir(env, 0777);
  snprintf(path, MAX_PATHLEN, "%s" IPTUX_PATH, env);
  if (access(path, F_OK) != 0) mkdir(path, 0777);
  snprintf(path, MAX_PATHLEN, "%s" LOG_PATH, env);
  if (access(path, F_OK) != 0) mkdir(path, 0777);
  snprintf(path, MAX_PATHLEN, "%s" PHOTO_PATH, env);
  if (access(path, F_OK) != 0) mkdir(path, 0777);
  snprintf(path, MAX_PATHLEN, "%s" ICON_PATH, env);
  if (access(path, F_OK) != 0) mkdir(path, 0777);
  snprintf(path, MAX_PATHLEN, "%s" LOG_PATH, env);
  if (access(path, F_OK) != 0) mkdir(path, 0777);
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
  GdkPixbuf *tpixbuf;

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
    tpixbuf = *pixbuf;
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
  gchar **uris, **ptr, *uri;
  GSList *filelist;

  if (!(uris = gtk_selection_data_get_uris(data))) return NULL;

  filelist = NULL;
  ptr = uris;
  while (*ptr) {
    uri = g_uri_unescape_string(*ptr, NULL);
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
 * 获取局域网网段名称.
 * @param ipv4 ipv4
 * @return name
 */
char *ipv4_get_lan_name(in_addr_t ipv4) {
  /**
   * @note 局域网网段划分，每两个为一组，以NULL标识结束.
   */
  const char *localgroup[] = {
      "10.0.0.0",    "10.255.255.255",  "172.16.0.0", "172.31.255.255",
      "192.168.0.0", "192.168.255.255", NULL};
  in_addr_t startip, endip;
  uint8_t count;
  char *ipstr;

  ipv4 = ntohl(ipv4);
  ipstr = NULL;

  count = 0;
  while (localgroup[count << 1]) {
    inet_pton(AF_INET, localgroup[count << 1], &startip);
    startip = ntohl(startip);
    inet_pton(AF_INET, localgroup[(count << 1) + 1], &endip);
    endip = ntohl(endip);
    ipv4_order(&startip, &endip);
    if (startip <= ipv4 && endip >= ipv4) {
      ipstr = g_strdup_printf("%s~%s", localgroup[count << 1],
                              localgroup[(count << 1) + 1]);
      break;
    }
    count++;
  }

  return ipstr;
}

/**
 * 让套接口支持广播.
 * @param sock socket
 */
void socket_enable_broadcast(int sock) {
  socklen_t len;
  int optval;

  optval = 1;
  len = sizeof(optval);
  if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &optval, len) != 0) {
    LOG_WARN("setsockopt for SO_BROADCAST failed: %s", strerror(errno));
  }
}

/**
 * 让套接口监听端口可重用.
 * @param sock socket
 */
void socket_enable_reuse(int sock) {
  socklen_t len;
  int optval;

  optval = 1;
  len = sizeof(optval);
  if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, len) != 0) {
    LOG_WARN("setsockopt for SO_REUSEADDR failed: %s", strerror(errno));
  }
}

/**
 * 获取系统主机的广播地址.
 * @param sock socket
 * @return 广播地址链表
 * @note 链表数据不是指针而是实际的IP
 */
GSList *get_sys_broadcast_addr(int sock) {
  const uint8_t amount = 5;  //支持5个IP地址
  uint8_t count, sum;
  struct ifconf ifc;
  struct ifreq *ifr;
  struct sockaddr_in *addr;
  GSList *list;

  list = g_slist_append(NULL, GUINT_TO_POINTER(inet_addr("255.255.255.255")));
  ifc.ifc_len = amount * sizeof(struct ifreq);
  ifc.ifc_buf = (caddr_t)g_malloc(ifc.ifc_len);
  if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
    g_free(ifc.ifc_buf);
    return list;
  }

  sum = ifc.ifc_len / sizeof(struct ifreq);
  count = 0;
  while (count < sum) {
    ifr = ifc.ifc_req + count;
    count++;

    if (ioctl(sock, SIOCGIFFLAGS, ifr) == -1 ||
        !(ifr->ifr_flags & IFF_BROADCAST) ||
        ioctl(sock, SIOCGIFBRDADDR, ifr) == -1)
      continue;
    addr = (struct sockaddr_in *)&ifr->ifr_broadaddr;
    list = g_slist_append(list, GUINT_TO_POINTER(addr->sin_addr.s_addr));
  }
  g_free(ifc.ifc_buf);

  return list;
}

/**
 * 获取系统主机的IP地址.
 * @param sock socket
 * @return IP地址链表
 * @note 链表数据不是指针而是实际的IP
 */
GSList *get_sys_host_addr(int sock) {
  const uint8_t amount = 5;  //支持5个IP地址
  uint8_t count, sum;
  struct ifconf ifc;
  struct ifreq *ifr;
  struct sockaddr_in *addr;
  GSList *list;

  list = NULL;
  ifc.ifc_len = amount * sizeof(struct ifreq);
  ifc.ifc_buf = (caddr_t)g_malloc(ifc.ifc_len);
  if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
    g_free(ifc.ifc_buf);
    return list;
  }

  sum = ifc.ifc_len / sizeof(struct ifreq);
  count = 0;
  while (count < sum) {
    ifr = ifc.ifc_req + count;
    count++;

    if (strncasecmp(ifr->ifr_name, "lo", 2) == 0 ||
        ioctl(sock, SIOCGIFFLAGS, ifr) == -1 || !(ifr->ifr_flags & IFF_UP) ||
        ioctl(sock, SIOCGIFADDR, ifr) == -1)
      continue;
    addr = (struct sockaddr_in *)&ifr->ifr_broadaddr;
    list = g_slist_append(list, GUINT_TO_POINTER(addr->sin_addr.s_addr));
  }
  g_free(ifc.ifc_buf);

  return list;
}

/**
 * 获取系统主机的IP地址串描述
 * @param sock socket
 * @return 描述串
 */
char *get_sys_host_addr_string(int sock) {
  char ipstr[INET_ADDRSTRLEN], *buf, *ptr;
  GSList *list, *tlist;
  uint16_t len;

  if (!(tlist = list = get_sys_host_addr(sock))) return NULL;

  len = g_slist_length(list) * INET_ADDRSTRLEN;
  ptr = buf = (char *)g_malloc(len);
  while (tlist) {
    inet_ntop(AF_INET, &tlist->data, ipstr, INET_ADDRSTRLEN);
    snprintf(ptr, len, "%s\n", ipstr);
    ptr += strlen(ptr);
    len -= INET_ADDRSTRLEN;
    tlist = g_slist_next(tlist);
  }
  *(ptr - 1) = '\0';  //抹除最后一个换行符
  g_slist_free(list);

  return buf;
}

}  // namespace iptux
