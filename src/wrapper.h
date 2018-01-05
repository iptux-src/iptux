//
// C++ Interface: wrapper
//
// Description:打包函数，使某些函数更加好用
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef WRAPPER_H
#define WRAPPER_H

#include "sys.h"

ssize_t xwrite(int fd, const void *buf, size_t count);
ssize_t xread(int fd, void *buf, size_t count);
ssize_t read_ipmsg_prefix(int fd, void *buf, size_t count);
ssize_t read_ipmsg_filedata(int fd, void *buf, size_t count, size_t offset);
ssize_t read_ipmsg_dirfiles(int fd, void *buf, size_t count, size_t offset);
ssize_t read_ipmsg_fileinfo(int fd, void *buf, size_t count, size_t offset);

#endif
