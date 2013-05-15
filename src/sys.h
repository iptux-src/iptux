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

/* <stdint.h> */
#define __STDC_LIMIT_MACROS
/* <inttypes.h> */
#define __STDC_FORMAT_MACROS

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
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
#include <sys/queue.h>
#include <fcntl.h>
#include <dirent.h>
#include <ftw.h>
#include <signal.h>
// #include <sys/vfs.h>
#include <pwd.h>
#include <getopt.h>
#include <locale.h>
#include <libintl.h>

#ifndef _
#define _(string) gettext(string)
#endif

#ifdef __APPLE__
#  define O_LARGEFILE 0
#endif

#endif
