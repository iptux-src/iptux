//
// C++ Implementation: CoreThread
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "CoreThread.h"
#include "MainWindow.h"
#include "StatusIcon.h"
#include "Control.h"
#include "SendFile.h"
#include "UdpData.h"
#include "TcpData.h"
#include "Command.h"
#include "baling.h"
#include "output.h"
#include "support.h"
#include "utils.h"

bool CoreThread::server = true;
CoreThread::CoreThread()
{
}

CoreThread::~CoreThread()
{
	extern struct interactive inter;

	server = false;
	shutdown(inter.udpsock, SHUT_RDWR);
	shutdown(inter.tcpsock, SHUT_RDWR);
}

void CoreThread::CoreThreadEntry()
{
	thread_create(ThreadFunc(RecvUdp), NULL, false);
	thread_create(ThreadFunc(RecvTcp), NULL, false);
	thread_create(ThreadFunc(WatchIptux), NULL, false);
	NotifyAll();
}

void CoreThread::NotifyAll()
{
	extern struct interactive inter;
	Command cmd;

	cmd.BroadCast(inter.udpsock);
	cmd.DialUp(inter.udpsock);
}

void CoreThread::RecvUdp()
{
	extern struct interactive inter;
	extern UdpData udt;
	char buf[MAX_UDPBUF];
	ssize_t size;
	socklen_t len;
	SI addr;

	while (server) {
		len = sizeof(addr);
		if ((size = recvfrom(inter.udpsock, buf, MAX_UDPBUF, 0,
				     (SA *) & addr, &len)) == -1)
			continue;
		if (size != MAX_UDPBUF)
			buf[size] = '\0';
		udt.UdpDataEntry(addr.sin_addr.s_addr, buf, size);
		pmessage(_("accept %s> %s\n"), inet_ntoa(addr.sin_addr), buf);
	}
}

void CoreThread::RecvTcp()
{
	extern struct interactive inter;
	int subsock;

	listen(inter.tcpsock, 5);
	while (server) {
		if ((subsock = Accept(inter.tcpsock, NULL, NULL)) == -1)
			continue;
		thread_create(ThreadFunc(TcpData::TcpDataEntry),
			      GINT_TO_POINTER(subsock), false);
	}
}

void CoreThread::WatchIptux()
{
	extern Control ctr;
	extern SendFile sfl;
	extern UdpData udt;

	while (server) {
		my_delay(1, 0);
		gdk_threads_enter();
		StatusIcon::UpdateTips();
		MainWindow::UpdateTips();
		gdk_threads_leave();
		ctr.AdjustMemory();
		udt.AdjustMemory();
		if (ctr.dirty)
			ctr.WriteControl();
		if (sfl.dirty)
			sfl.WriteShared();
	}
}
