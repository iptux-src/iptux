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
#include "config.h"
#include "Models.h"

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
      packetn(0),
      rpacketn(0),
      flags(0) {}
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

bool PalInfo::isCompatible() const {
  return FLAG_ISSET(this->flags, 0);
}

bool PalInfo::isOnline() const {
  return FLAG_ISSET(this->flags, 1);
}

bool PalInfo::isChanged() const {
  return FLAG_ISSET(this->flags, 2);
}

void PalInfo::setCompatible(bool value) {
  if(value) {
    FLAG_SET(this->flags, 0);
  } else {
    FLAG_CLR(this->flags, 0);
  }
}

void PalInfo::setOnline(bool value) {
  if(value) {
    FLAG_SET(this->flags, 1);
  } else {
    FLAG_CLR(this->flags, 1);
  }
}

void PalInfo::setChanged(bool value) {
  if(value) {
    FLAG_SET(this->flags, 2);
  } else {
    FLAG_CLR(this->flags, 2);
  }
}

FileInfo::FileInfo()
    : fileid(0),
      packetn(0),
      fileattr(0),
      filesize(-1),
      finishedsize(0),
      fileown(NULL),
      filepath(NULL),
      filectime(0),
      filemtime(0),
      filenum(0) {}
FileInfo::~FileInfo() { g_free(filepath); }

MsgPara::MsgPara()
    : pal(NULL),
      stype(MessageSourceType::PAL),
      btype(GROUP_BELONG_TYPE_REGULAR) {}

MsgPara::~MsgPara() {
}

ChipData::ChipData() : type(MESSAGE_CONTENT_TYPE_STRING), data("") {}
ChipData::~ChipData() {}

NetSegment::NetSegment() {}
NetSegment::~NetSegment() {}

bool NetSegment::ContainIP(in_addr_t ipv4) const {
  ipv4 = ntohl(ipv4);

  in_addr_t startip2, endip2;
  inet_pton(AF_INET, startip.c_str(), &startip2);
  startip2 = ntohl(startip2);
  inet_pton(AF_INET, endip.c_str(), &endip2);
  endip2 = ntohl(endip2);
  ipv4_order(&startip2, &endip2);
  return ipv4 >= startip2 && ipv4 <= endip2;
}

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
  res.endip = value["endip"].asString();
  res.description = value["description"].asString();
  return res;
}

SessionAbstract::SessionAbstract() {}
SessionAbstract::~SessionAbstract() {}

}  // namespace iptux
