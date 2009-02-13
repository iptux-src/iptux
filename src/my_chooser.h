//
// C++ Interface: my_chooser
//
// Description:附带预览功能的文件选择器
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MY_CHOOSER_H
#define MY_CHOOSER_H

#include "face.h"

class my_chooser {
 public:
	my_chooser(const gchar * t, GtkWidget * p);
	~my_chooser();

	static gchar *choose_file(const gchar * t, GtkWidget * p);
 private:
	void create_chooser();
	gchar *run_chooser();

	GtkWidget *chooser;
	GtkWidget *parent;
	const gchar *title;
//回调处理部分
 private:
	static void UpdatePreview(GtkFileChooser * chooser,
				  GtkWidget * preview);
};

#endif
