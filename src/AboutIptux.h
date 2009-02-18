//
// C++ Interface: AboutIptux
//
// Description:创建关于对话框
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ABOUTIPTUX_H
#define ABOUTIPTUX_H

#include "face.h"

class AboutIptux {
 public:
	AboutIptux();
	~AboutIptux();

	static void AboutEntry();
	static void MoreEntry();
 private:
	static void CreateAbout();
	void CreateMore();
	void RunDialog(GtkWidget **dialog);
	static bool CheckExist(GtkWidget *dialog);

    static void show_help_page(GtkTextBuffer *buffer, const char * str);

	static GtkWidget *about;
	static GtkWidget *more;
//回调处理部分
 private:
	 static void DialogDestroy(GtkWidget **dialog);
     static void activate_url (GtkAboutDialog *about,
	      const gchar    *link,
	      gpointer        data);
};

#endif
