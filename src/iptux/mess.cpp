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

#include "iptux/utils.h"

namespace iptux {

PalInfo::PalInfo()
    : ipv4(0),
      segdes(NULL),
      version(NULL),
      user(NULL),
      host(NULL),
      name(NULL),
      group(NULL),
      photo(NULL),
      sign(NULL),
      iconfile(NULL),
      encode(NULL),
      flags(0),
      packetn(0),
      rpacketn(0) {}
PalInfo::~PalInfo() {
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

GroupInfo::GroupInfo()
    : grpid(0),
      type(GROUP_BELONG_TYPE_REGULAR),
      name(NULL),
      member(NULL),
      buffer(NULL),
      dialog(NULL) {}
GroupInfo::~GroupInfo() {
  g_free(name);
  g_slist_free(member);
  g_object_unref(buffer);
}

FileInfo::FileInfo()
    : fileid(0),
      packetn(0),
      fileattr(0),
      filesize(-1),
      finishedsize(0),
      fileown(NULL),
      filepath(NULL) {}
FileInfo::~FileInfo() { g_free(filepath); }

MsgPara::MsgPara()
    : pal(NULL),
      stype(MessageSourceType::PAL),
      btype(GROUP_BELONG_TYPE_REGULAR),
      dtlist(NULL) {}
MsgPara::~MsgPara() {
  for (GSList* tlist = dtlist; tlist; tlist = g_slist_next(tlist))
    delete (ChipData*)tlist->data;
  g_slist_free(dtlist);
}

ChipData::ChipData() : type(MESSAGE_CONTENT_TYPE_STRING), data(NULL) {}
ChipData::~ChipData() { g_free(data); }

NetSegment::NetSegment() {}
NetSegment::~NetSegment() {}

Json::Value NetSegment::ToJsonValue() const {
  Json::Value value;
  value["startip"] = startip;
  value["endip"] = endip;
  value["description"] = description;
  return value;
}

NetSegment NetSegment::fromJsonValue(Json::Value &value) {
  NetSegment res;
  res.startip = value["startip"].asString();
  res.endip = value["startip"].asString();
  res.description = value["startip"].asString();
  return res;
}

SessionAbstract::SessionAbstract() {}
SessionAbstract::~SessionAbstract() {}

TransAbstract::TransAbstract() {}
TransAbstract::~TransAbstract() {}

}  // namespace iptux
