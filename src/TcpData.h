//
// C++ Interface: TcpData
//
// Description:对TCP连接进行处理
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TCPDATA_H
#define TCPDATA_H

#include "sys.h"

class TcpData {
 public:
	TcpData();
	~TcpData();

	static void TcpDataEntry(int sock);
 private:
	static void RecvSublayer(int sock, uint32_t command, char *buf,
				 ssize_t size);
	static void RecvSublayerData(int sock, int fd, char *buf, ssize_t size);
};

#endif
