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
 private:
	void CreateAbout();
	void RunAbout();
	static bool CheckExist();

	static GtkWidget *about;
//回调处理部分
 private:
	static void AboutDestroy();
};

#endif
