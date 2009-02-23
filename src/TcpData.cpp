//
// C++ Implementation: TcpData
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "TcpData.h"
#include "SendFile.h"
#include "UdpData.h"
#include "Pal.h"
#include "udt.h"
#include "baling.h"
#include "utils.h"

TcpData::TcpData()
{
}

TcpData::~TcpData()
{
}

void TcpData::TcpDataEntry(int sock)
{
	extern SendFile sfl;
	char buf[MAX_SOCKBUF];
	uint32_t commandno;
	ssize_t size;

	size = read_ipmsg_prefix(sock, buf, MAX_SOCKBUF, 5);
	if (size <= 0) {
		close(sock);
		return;
	}

	commandno = iptux_get_dec_number(buf, 4);
	switch (GET_MODE(commandno)) {
	case IPMSG_GETFILEDATA:
		sfl.RequestData(sock, IPMSG_FILE_REGULAR, buf);
		break;
	case IPMSG_GETDIRFILES:
		sfl.RequestData(sock, IPMSG_FILE_DIR, buf);
		break;
	case IPTUX_SENDSUBLAYER:
		RecvSublayer(sock, commandno, buf, size);
		break;
	default:
		break;
	}

	close(sock);
}

void TcpData::RecvSublayer(int sock, uint32_t command, char *buf, ssize_t size)
{
	extern UdpData udt;
	socklen_t len;
	SI addr;
	char path[MAX_PATHBUF];
	uint32_t packetno;
	Pal *pal;
	int fd;

	len = sizeof(addr);
	getpeername(sock, (SA *) & addr, &len);
	if (!(pal = (Pal *) udt.Ipv4GetPal(addr.sin_addr.s_addr)))
		return;

	packetno = iptux_get_dec_number(buf, 1);
	switch (GET_OPT(command)) {
	case IPTUX_ADPICOPT:
		snprintf(path, MAX_PATHBUF, "%s" ADS_PATH "/%" PRIx32,
			 g_get_user_cache_dir(), pal->Ipv4Quote());
		break;
	case IPTUX_MSGPICOPT:
		snprintf(path, MAX_PATHBUF, "%s" PIC_PATH "/%" PRIx32
			 "-%" PRIx32 "-%" PRIx32, g_get_user_cache_dir(),
			 pal->Ipv4Quote(), packetno, time(NULL));
		break;
	default:
		snprintf(path, MAX_PATHBUF, "%s" IPTUX_PATH "/%" PRIx32 "-%"
			 PRIx32 "-%" PRIx32, g_get_user_cache_dir(),
			 pal->Ipv4Quote(), packetno, time(NULL));
		break;
	}
	if ((fd = Open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
		return;

	RecvSublayerData(sock, fd, buf, size);
	close(fd);
	udt.SublayerEntry(pal, command, path);
}

void TcpData::RecvSublayerData(int sock, int fd, char *buf, ssize_t size)
{
	size_t len;

	len = strlen(buf) + 1;
	if (size != len)
		Write(fd, buf + len, size - len);
	do {
		size = Read(sock, buf, MAX_SOCKBUF);
		if (size == 0 || size == -1)
			break;
		size = Write(fd, buf, size);
		if (size == -1)
			break;
	} while (1);
}
