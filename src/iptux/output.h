//
// C++ Interface: output
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef OUTPUT_H
#define OUTPUT_H

#include "deplib.h"

/* 警告信息输出 */
#ifndef WARNING
#define pwarning(format,...) /*warnx(format,##__VA_ARGS__)*/
#else
#define pwarning(format,...) warnx(format,##__VA_ARGS__)
#endif

/* 常规消息输出 */
#ifndef MESSAGE
#define pmessage(format,...) /*printf(format,##__VA_ARGS__)*/
#else
#define pmessage(format,...) printf(format,##__VA_ARGS__)
#endif

/* 程序执行踪迹输出，用于调试 */
#ifndef TRACE
#define ptrace(format,...) /*printf(format,##__VA_ARGS__)*/
#else
#define ptrace(format,...) printf(format,##__VA_ARGS__)
#endif

void pop_info(GtkWidget *parent, const gchar *format, ...);
void pop_warning(GtkWidget *parent, const gchar *format, ...);
void pop_error(const gchar *format, ...);

#endif
