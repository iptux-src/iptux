//
// C++ Implementation: mess
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "mess.h"
#include "utils.h"

PalInfo::PalInfo():ipv4(0), segdes(NULL), version(NULL), user(NULL), host(NULL),
 name(NULL), group(NULL), photo(NULL), sign(NULL), iconfile(NULL), encode(NULL),
 flags(0), packetn(0), rpacketn(0)
{}
PalInfo::~PalInfo()
{
	g_free(segdes);
	g_free(version);
	g_free(user);
	g_free(host);
	g_free(name);
	g_free(group);
	g_free(photo);
	g_free(sign);
	g_free(iconfile);
	g_free(encode);
}

GroupInfo::GroupInfo():grpid(0), type(REGULAR_TYPE), name(NULL),
 member(NULL), buffer(NULL), dialog(NULL)
{}
GroupInfo::~GroupInfo()
{
	g_free(name);
	g_slist_free(member);
	g_object_unref(buffer);
}

FileInfo::FileInfo():fileid(0), packetn(0), fileattr(0), filesize(-1),
 fileown(NULL), filepath(NULL)
{}
FileInfo::~FileInfo()
{
	g_free(filepath);
}

MsgPara::MsgPara():pal(NULL), stype(PAL_TYPE), btype(REGULAR_TYPE), dtlist(NULL)
{}
MsgPara::~MsgPara()
{
	for (GSList *tlist = dtlist; tlist; tlist = g_slist_next(tlist))
		delete (ChipData *)tlist->data;
	g_slist_free(dtlist);
}

ChipData::ChipData():type(STRING_TYPE), data(NULL)
{}
ChipData::~ChipData()
{
	g_free(data);
}

NetSegment::NetSegment():startip(NULL), endip(NULL), description(NULL)
{}
NetSegment::~NetSegment()
{
	g_free(startip);
	g_free(endip);
	g_free(description);
}

SessionAbstract::SessionAbstract()
{}
SessionAbstract::~SessionAbstract()
{}

TransAbstract::TransAbstract()
{}
TransAbstract::~TransAbstract()
{}
