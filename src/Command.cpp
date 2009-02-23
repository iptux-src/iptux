//
// C++ Implementation: Command
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Command.h"
#include "Control.h"
#include "Pal.h"
#include "utils.h"
#include "support.h"
#include "baling.h"

uint32_t Command::packetn = 0;
 Command::Command():size(0)
{
}

Command::~Command()
{
}

//广播
void Command::BroadCast(int sock)
{
	extern struct interactive inter;
	extern Control ctr;
	GSList *list, *tmp;
	SI addr;

	CreateCommand(IPMSG_ABSENCEOPT | IPMSG_BR_ENTRY, ctr.myname);
	TransferEncode(ctr.encode);
	CreateIptuxExtra(ctr.encode);

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPTUX_DEFAULT_PORT);
	list = tmp = get_sys_broadcast_addr(inter.udpsock);
	while (tmp) {
		addr.sin_addr.s_addr = GPOINTER_TO_UINT(tmp->data);
		sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
		my_delay(0, 9999999);
		tmp = tmp->next;
	}
	g_slist_free(list);
}

//单点广播
void Command::DialUp(int sock)
{
	extern Control ctr;
	in_addr_t ip1, ip2, ip;
	NetSegment *ns;
	GSList *list, *tmp;
	SI addr;

	CreateCommand(IPMSG_DIALUPOPT | IPMSG_ABSENCEOPT | IPMSG_BR_ENTRY,
							      ctr.myname);
	TransferEncode(ctr.encode);
	CreateIptuxExtra(ctr.encode);

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPTUX_DEFAULT_PORT);
	list = tmp = ctr.CopyNetSegment();	//与获取网段描述字冲突，必须复制出来使用
	while (tmp) {
		ns = (NetSegment *) tmp->data;
		inet_pton(AF_INET, ns->startip, &ip1);
		inet_pton(AF_INET, ns->endip, &ip2);
		ip1 = ntohl(ip1), ip2 = ntohl(ip2);
		ipv4_order(&ip1, &ip2);
		ip = ip1;
		while (ip <= ip2) {
			addr.sin_addr.s_addr = htonl(ip++);
			sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
			my_delay(0, 999999);
		}
		tmp = tmp->next;
	}
	g_slist_foreach(list, GFunc(remove_foreach),
			GINT_TO_POINTER(NETSEGMENT));
	g_slist_free(list);
}

//通知在线
void Command::SendAnsentry(int sock, pointer data)
{
	extern Control ctr;
	Pal *pal;
	SI addr;

	pal = (Pal *) data;
	CreateCommand(IPMSG_ABSENCEOPT | IPMSG_ANSENTRY, ctr.myname);
	TransferEncode(pal->EncodeQuote());
	CreateIptuxExtra(pal->EncodeQuote());

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPTUX_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->Ipv4Quote();

	sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
}

//通告退出
void Command::SendExit(int sock, pointer data)
{
	Pal *pal;
	SI addr;

	pal = (Pal *) data;
	CreateCommand(IPMSG_DIALUPOPT | IPMSG_BR_EXIT, NULL);
	TransferEncode(pal->EncodeQuote());

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPTUX_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->Ipv4Quote();

	sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
}

//更改我的信息
void Command::SendAbsence(int sock, pointer data)
{
	extern Control ctr;
	Pal *pal;
	SI addr;

	pal = (Pal *) data;
	CreateCommand(IPMSG_ABSENCEOPT | IPMSG_BR_ABSENCE, ctr.myname);
	TransferEncode(pal->EncodeQuote());
	CreateIptuxExtra(pal->EncodeQuote());

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPTUX_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->Ipv4Quote();

	sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
}

//探测好友
void Command::SendDetectPacket(int sock, in_addr_t ipv4)
{
	extern Control ctr;
	SI addr;

	CreateCommand(IPMSG_DIALUPOPT | IPMSG_ABSENCEOPT | IPMSG_BR_ENTRY,
							      ctr.myname);
	TransferEncode(ctr.encode);
	CreateIptuxExtra(ctr.encode);

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPTUX_DEFAULT_PORT);
	addr.sin_addr.s_addr = ipv4;

	sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
}

//发送消息
void Command::SendMessage(int sock, pointer data, const char *msg)
{
	uint32_t packetno;
	uint8_t count;
	GSList *chiplist;
	Pal *pal;
	SI addr;

	pal = (Pal *) data, packetno = packetn;
	pal->CheckReply(packetno, true);
	CreateCommand(IPMSG_SENDCHECKOPT | IPMSG_SENDMSG, msg);
	TransferEncode(pal->EncodeQuote());

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPTUX_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->Ipv4Quote();

	count = 0;
	do {
		sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
		my_delay(1, 0);
		count++;
	} while (!pal->CheckReply(packetno, false) && count < MAX_RETRYTIMES);
	if (count >= MAX_RETRYTIMES) {
		chiplist = g_slist_append(NULL, new ChipData(STRING,
			  _("It's offline, or not receive the data packet!")));
		pal->BufferInsertData(chiplist, ERROR);
		g_slist_foreach(chiplist, GFunc(remove_foreach),
				GINT_TO_POINTER(UNKNOWN));	//静态内存，不需析构释放
		g_slist_free(chiplist);
	}
}

//回复消息
void Command::SendReply(int sock, pointer data, uint32_t packetno)
{
	char packetstr[11];	//uint32_t 10进制字符串长为 10
	Pal *pal;
	SI addr;

	pal = (Pal *) data;
	snprintf(packetstr, 11, "%" PRIu32, packetno);
	CreateCommand(IPMSG_SENDCHECKOPT | IPMSG_RECVMSG, packetstr);
	TransferEncode(pal->EncodeQuote());

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPTUX_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->Ipv4Quote();

	sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
}

//群发消息
void Command::SendGroupMsg(int sock, pointer data, const char *msg)
{
	Pal *pal;
	SI addr;

	pal = (Pal *) data;
	CreateCommand(IPMSG_BROADCASTOPT | IPMSG_SENDMSG, msg);
	TransferEncode(pal->EncodeQuote());

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPTUX_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->Ipv4Quote();

	sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
}

bool Command::SendAskData(int sock, pointer data, uint32_t packetno,
			  uint32_t fileid, uint64_t offset)
{
	char attrstr[43];	//10+1+10+1+20 +1	=43
	Pal *pal;
	SI addr;

	pal = (Pal *) data;
	snprintf(attrstr, 43, "%" PRIx32 ":%" PRIx32 ":%" PRIx64,
						 packetno, fileid, offset);
	CreateCommand(IPMSG_FILEATTACHOPT | IPMSG_GETFILEDATA, attrstr);
	TransferEncode(pal->EncodeQuote());

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPTUX_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->Ipv4Quote();

	if (Connect(sock, (SA *) & addr, sizeof(addr)) == -1)
		return false;
	if (Write(sock, buf, size) == -1)
		return false;

	return true;
}

bool Command::SendAskFiles(int sock, pointer data, uint32_t packetno,
			   uint32_t fileid)
{
	char attrstr[24];	//10+1+10+1+1 +1	=24
	Pal *pal;
	SI addr;

	pal = (Pal *) data;
	snprintf(attrstr, 24, "%" PRIx32 ":%" PRIx32 ":0", packetno, fileid);	//兼容 LanQQ 软件
	CreateCommand(IPMSG_FILEATTACHOPT | IPMSG_GETDIRFILES, attrstr);
	TransferEncode(pal->EncodeQuote());

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPTUX_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->Ipv4Quote();

	if (Connect(sock, (SA *) & addr, sizeof(addr)) == -1)
		return false;
	if (Write(sock, buf, size) == -1)
		return false;

	return true;
}

void Command::SendAskShared(int sock, pointer data, uint32_t opttype,
			    const char *extra)
{
	Pal *pal;
	SI addr;

	pal = (Pal *) data;
	CreateCommand(opttype | IPTUX_ASKSHARED, extra);
	TransferEncode(pal->EncodeQuote());

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPTUX_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->Ipv4Quote();

	sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
}

void Command::SendFileInfo(int sock, pointer data, uint32_t opttype,
			    const char *extra)
{
	char *ptr;
	Pal *pal;
	SI addr;

	pal = (Pal *) data;
	CreateCommand(opttype | IPMSG_FILEATTACHOPT | IPMSG_SENDMSG, NULL);
	TransferEncode(pal->EncodeQuote());
	ptr = transfer_encode(extra, pal->EncodeQuote(), true);
	CreateIpmsgExtra(ptr);
	free(ptr);

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPTUX_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->Ipv4Quote();

	sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
}

void Command::SendMyIcon(int sock, pointer data)
{
	Pal *pal;
	SI addr;

	pal = (Pal *) data;
	CreateCommand(IPTUX_SENDICON, NULL);
	TransferEncode(pal->EncodeQuote());
	CreateIconExtra();

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPTUX_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->Ipv4Quote();

	sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
}

void Command::SendMySign(int sock, pointer data)
{
	extern Control ctr;
	Pal *pal;
	SI addr;

	pal = (Pal *) data;
	CreateCommand(IPTUX_SENDSIGN, ctr.sign);
	TransferEncode(pal->EncodeQuote());

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPTUX_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->Ipv4Quote();

	sendto(sock, buf, size, 0, (SA *) & addr, sizeof(addr));
}

void Command::SendSublayer(int sock, pointer data, uint32_t opttype,
			   const char *path)
{
	Pal *pal;
	SI addr;
	int fd;

	pal = (Pal *) data;
	CreateCommand(opttype | IPTUX_SENDSUBLAYER, NULL);
	TransferEncode(pal->EncodeQuote());

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPTUX_DEFAULT_PORT);
	addr.sin_addr.s_addr = pal->Ipv4Quote();

	if (Connect(sock, (SA *) & addr, sizeof(addr)) == -1
		   || Write(sock, buf, size) == -1
		   || (fd = Open(path, O_RDONLY)) == -1)
		return;

	SendSublayerData(sock, fd);
	close(fd);
}

void Command::SendSublayerData(int sock, int fd)
{
	ssize_t len;

	do {
		len = Read(fd, buf, MAX_UDPBUF);
		if (len == 0 || len == -1)
			break;
		len = Write(sock, buf, len);
		if (len == -1)
			break;
	} while (1);
}

void Command::CreateCommand(uint32_t command, const char *attach)
{
	const gchar *env;
	char *ptr;

	snprintf(buf, MAX_UDPBUF, "%s", IPTUX_VERSION);
	size = strlen(buf);
	ptr = buf + size;

	snprintf(ptr, MAX_UDPBUF - size, ":%" PRIu32, packetn++);
	size += strlen(ptr);
	ptr = buf + size;

	env = g_get_user_name();
	snprintf(ptr, MAX_UDPBUF - size, ":%s", env);
	size += strlen(ptr);
	ptr = buf + size;

	env = g_get_host_name();
	snprintf(ptr, MAX_UDPBUF - size, ":%s", env);
	size += strlen(ptr);
	ptr = buf + size;

	snprintf(ptr, MAX_UDPBUF - size, ":%" PRIu32, command);
	size += strlen(ptr);
	ptr = buf + size;

	snprintf(ptr, MAX_UDPBUF - size, ":%s", attach ? attach : "");
	size += strlen(ptr) + 1;
}

void Command::TransferEncode(const char *encode)
{
	char *ptr;

	ptr = transfer_encode(buf, encode, true);
	size = strlen(ptr) + 1;
	memcpy(buf, ptr, size);
	free(ptr);
}

void Command::CreateIptuxExtra(const char *encode)
{
	extern Control ctr;
	char *ptr, *tmp;

	ptr = buf + size;
	tmp = transfer_encode(ctr.mygroup, encode, true);
	snprintf(ptr, MAX_UDPBUF - size, "%s", tmp);
	free(tmp);
	size += strlen(ptr) + 1;

	ptr = buf + size;
	tmp = strrchr(ctr.myicon, '/');
	snprintf(ptr, MAX_UDPBUF - size, "%s", tmp ? tmp + 1 : ctr.myicon);
	size += strlen(ptr) + 1;

	ptr = buf + size;
	snprintf(ptr, MAX_UDPBUF - size, "UTF-8");
	size += strlen(ptr) + 1;
}

void Command::CreateIpmsgExtra(const char *extra)
{
	char *ptr, *tmp;

	ptr = buf + size;
	snprintf(ptr, MAX_UDPBUF - size, "%s", extra);
	if ( (tmp = strrchr(ptr, '\a')) )
		*(tmp + 1) = '\0';
	size += strlen(ptr) + 1;
}

void Command::CreateIconExtra()
{
	const gchar *env;
	char path[MAX_PATHBUF];
	ssize_t len;
	int fd;

	env = g_get_user_config_dir();
	snprintf(path, MAX_PATHBUF, "%s" COMPLEX_PATH "/icon", env);
	if ((fd = Open(path, O_RDONLY)) == -1)
		return;
	len = Read(fd, buf + size, MAX_UDPBUF - size);
	close(fd);
	if (len != -1)
		size += len;
}
