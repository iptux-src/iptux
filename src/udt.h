//
// C++ Interface: udt
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef UDT_H
#define UDT_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "sys.h"
#include "net.h"
#include "face.h"
#include "ipmsg.h"

enum BELONG_TYPE {
	PAL,
	SELF,
	ERROR,
};

enum INFO_TYPE {
	PALINFO,
	SYSICON,
	FILEINFO,
	CHIPDATA,
	NETSEGMENT,
	UNKNOWN
};

enum STATE_TYPE {
	Success,
	Fail,
	Quit
};

enum MSG_TYPE {
	STRING,
	PICTURE
};

struct interactive {
	GtkWidget *window;
	GtkStatusIcon *status_icon;
	int udpsock, tcpsock;
};

struct recvfile_para {
	gpointer data;
	char *msg;
	uint32_t commandn;
	uint32_t packetn;
};

struct sendmsg_para {
	gpointer data;
	GSList *chiplist;
};

class SysIcon {
public:
	SysIcon(char *path, GdkPixbuf *pix):
		pathname(path), pixbuf(pix) {
	} ~SysIcon() {
		free(pathname);
		if (pixbuf)
			g_object_unref(pixbuf);
	}
	char *pathname;
	GdkPixbuf *pixbuf;
};

class FileInfo {
 public:
	FileInfo(uint32_t id, char *name, uint64_t size,
		 uint32_t attr):fileid(id), filename(name),
	    filesize(size), fileattr(attr) {
	} ~FileInfo() {
		free(filename);
	}

	uint32_t fileid;
	char *filename;
	uint64_t filesize;
	uint32_t fileattr;
};

class ChipData {
 public:
	ChipData(enum MSG_TYPE tp, char *dt):type(tp), data(dt) {
	} ~ChipData() {
		free(data);
	}

	enum MSG_TYPE type;
	char *data;
};

class NetSegment {
 public:
	NetSegment(char *start, char *end, char *dsc):startip(start),
	    endip(end), describe(dsc) {
	} ~NetSegment() {
		free(startip);
		free(endip);
		free(describe);
	}

	char *startip;
	char *endip;
	char *describe;
};

typedef struct sockaddr SA;
typedef struct sockaddr_in SI;

#endif
