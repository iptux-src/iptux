//
// C++ Interface: DetectPal
//
// Description:探测好友
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DETECTPAL_H
#define DETECTPAL_H

#include "face.h"

class DetectPal {
 public:
	DetectPal();
	~DetectPal();

	static void DetectEntry();
 private:
	void CreateDetect();
	void RunDetect();
	void SendDetectPacket();
	static bool CheckExsit();

	GtkWidget *ipstr;
	static GtkWidget *detect;
//回调处理部分
 private:
	static void DetectDestroy();
};

#endif
