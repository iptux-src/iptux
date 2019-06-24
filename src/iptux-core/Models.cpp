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

#include <sstream>

#include "iptux-core/utils.h"
#include "iptux-core/ipmsg.h"

using namespace std;

namespace iptux {

PalInfo::PalInfo()
    : ipv4({0}),
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
      flags(0) {
       encode = g_strdup("");
      }
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
      filepath(NULL),
      filectime(0),
      filemtime(0),
      filenum(0) {}
FileInfo::~FileInfo() { g_free(filepath); }

FileInfo::FileInfo(const FileInfo& f)
  : fileid(f.fileid),
    packetn(f.packetn),
    fileattr(f.fileattr),
    filesize(f.filesize),
    finishedsize(f.finishedsize),
    fileown(f.fileown),
    filectime(f.filectime),
    filemtime(f.filemtime),
    filenum(f.filenum)
{
  filepath = g_strdup(f.filepath);
}

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

bool NetSegment::ContainIP(in_addr ipv4) const {
  return ipv4Compare(stringToInAddr(startip), ipv4) <= 0
    && ipv4Compare(ipv4, stringToInAddr(endip)) <= 0;
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

string ChipData::ToString() const {
  ostringstream oss;
  oss << "ChipData(";
  switch(type) {
  case MessageContentType::STRING:
    oss << "MessageContentType::STRING";
    break;
  case MessageContentType::PICTURE:
    oss << "MessageContentType::PICTURE";
    break;
  default:
    g_assert_not_reached();
  }
  oss << ", ";
  oss << data;
  oss << ")";
  return oss.str();
}

PalKey::PalKey(in_addr ipv4)
  : ipv4(ipv4), port(IPTUX_DEFAULT_PORT)
{}

PalKey::PalKey(in_addr ipv4, int port)
  : ipv4(ipv4), port(port)
{}

bool PalKey::operator==(const PalKey& rhs) const {
  return ipv4Equal(this->ipv4, rhs.ipv4)
    && this->port == rhs.port;
}

string PalKey::ToString() const {
  return stringFormat("%s:%d", inAddrToString(ipv4).c_str(), port);
}

}  // namespace iptux
