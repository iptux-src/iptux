//
// C++ Interface: Log
//
// Description:相关日志记录
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef LOG_H
#define LOG_H

#include "sys.h"

class Log {
 public:
	Log();
	~Log();

	void InitSelf();
	void CommunicateLog(pointer data, const char *fmt, ...);	//data pal|NULL
	void SystemLog(const char *fmt, ...);
 private:
	 FILE * communicate;
	FILE *system;
};

#endif
