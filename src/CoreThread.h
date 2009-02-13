//
// C++ Interface: CoreThread
//
// Description:程序中的核心线程
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CORETHREAD_H
#define CORETHREAD_H

class CoreThread {
 public:
	CoreThread();
	~CoreThread();

	static void CoreThreadEntry();
	static void NotifyAll();
 private:
	static void RecvUdp();
	static void RecvTcp();
	static void WatchIptux();

	static bool server;
};

#endif
