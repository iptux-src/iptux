//
// C++ Interface: sys
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SYS_H
#define SYS_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <err.h>
#include <time.h>
#include <iconv.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <dirent.h>
#include <ftw.h>
#include <signal.h>
#include <sys/vfs.h>
typedef void *pointer;

#ifdef HAVE_GST
#include <gst/gst.h>
#endif

#endif
