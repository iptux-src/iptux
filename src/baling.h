//
// C++ Interface: baling
//
// Description:打包函数，使某些函数更加好用
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef BALING_H
#define BALING_H

#include "sys.h"
#include "face.h"

//sys
char *Strdup(const char *str);
void *Malloc(size_t size);
void *operator      new(size_t size);

int Socket(int domain, int type, int protocol);
int Accept(int sockfd, struct sockaddr *addr, socklen_t * addr_len);
int Connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen);

int Stat(const char *path, struct stat64 *buf);
int Open(const char *pathname, int flags, ...);
ssize_t Read(int fd, void *buf, size_t count);
ssize_t Write(int fd, const void *buf, size_t count);
ssize_t read_ipmsg_prefix(int fd, void *buf, size_t count, uint8_t times);
ssize_t read_ipmsg_fileinfo(int fd, void *buf, size_t count, size_t offset);

FILE *Fopen(const char *path, const char *mode);
int Mkdir(const char *pathname, mode_t mode);

//gtk+
GtkWidget *create_box(gboolean vertical = TRUE);
GtkWidget *create_button_box(gboolean vertical = TRUE);
GtkWidget *create_button(const gchar * title);
GtkWidget *create_paned(gboolean vertical = TRUE);
GtkWidget *create_scrolled_window();
GtkWidget *create_label(const gchar * title);
GtkWidget *create_frame(const gchar * title);
GtkWidget *create_text_view();
GtkWidget *create_window(const gchar * title, gdouble width, gdouble height);

#endif
