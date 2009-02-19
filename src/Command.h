//
// C++ Interface: Command
//
// Description:创建命令并发送
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef COMMAND_H
#define COMMAND_H

#include "udt.h"

/*参数 data 类型为class Pal*/
class Command {
 public:
	Command();
	~Command();

	void BroadCast(int sock);
	void DialUp(int sock);
	void SendAnsentry(int sock, pointer data);
	void SendExit(int sock, pointer data);
	void SendAbsence(int sock, pointer data);
	void SendDetectPacket(int sock, in_addr_t ipv4);
	void SendMessage(int sock, pointer data, const char *msg);
	void SendReply(int sock, pointer data, uint32_t packetno);
	void SendGroupMsg(int sock, pointer data, const char *msg);

	bool SendAskData(int sock, pointer data, uint32_t packetno,
			 uint32_t fileid, uint64_t offset);
	bool SendAskFiles(int sock, pointer data, uint32_t packetno,
			  uint32_t fileid);
	void SendAskShared(int sock, pointer data, uint32_t opttype,
			  const char *extra);
	void SendFileInfo(int sock, pointer data, uint32_t opttype,
			  const char *extra);
	void SendMyIcon(int sock, pointer data);
	void SendMySign(int sock, pointer data);
	void SendSublayer(int sock, pointer data, uint32_t opttype,
			  const char *path);
 private:
	void SendSublayerData(int sock, int fd);
	void CreateCommand(uint32_t command, const char *attach);
	void TransferEncode(const char *encode);
	void CreateIptuxExtra(const char *encode);
	void CreateIpmsgExtra(const char *extra);
	void CreateIconExtra();

	char buf[MAX_UDPBUF];
	size_t size;
	static uint32_t packetn;
};

#endif
