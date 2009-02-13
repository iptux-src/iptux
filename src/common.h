//
// C++ Interface: common
//
// Description:全局变量、数据
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef COMMON_H
#define COMMON_H

#include "udt.h"
#include "Control.h"
#include "UdpData.h"
#include "SendFile.h"
#include "Transport.h"
#include "Log.h"
#include "MainWindow.h"

Control ctr;
UdpData udt;
SendFile sfl;
Transport trans;
Log mylog;
struct interactive inter;
MainWindow *mwp;	//主窗口类指针

#endif
