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
#include "iptux-core/Models.h"

#include <sstream>

#include "iptux-utils/utils.h"
#include "iptux-core/internal/ipmsg.h"

using namespace std;

namespace iptux {

PalInfo::PalInfo()
    : ipv4({0}),
      segdes(NULL),
      version(NULL),
      user(NULL),
      host(NULL),
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
      fileattr(FileAttr::UNKNOWN),
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

NetSegment::NetSegment(string startip, string endip, string description)
  : startip(startip), endip(endip), description(description)
{}

NetSegment::~NetSegment() {}

uint64_t NetSegment::Count() const {
  uint32_t start = inAddrToUint32(inAddrFromString(startip));
  uint32_t end = inAddrToUint32(inAddrFromString(endip));
  if(start > end) {
    return 0;
  }
  return uint64_t(end) - uint64_t(start) + 1;
}

std::string NetSegment::NthIp(uint64_t i) const {
  uint32_t start = inAddrToUint32(inAddrFromString(startip));
  uint64_t res = start + i;
  return inAddrToString(inAddrFromUint32(res));
}



bool NetSegment::ContainIP(in_addr ipv4) const {
  return ipv4Compare(inAddrFromString(startip), ipv4) <= 0
    && ipv4Compare(ipv4, inAddrFromString(endip)) <= 0;
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

bool FileInfo::operator==(const FileInfo& rhs) const {
  const FileInfo& lhs = *this;
  return lhs.fileid == rhs.fileid
    && lhs.packetn == rhs.packetn
    && lhs.fileattr == rhs.fileattr
    && lhs.filesize == rhs.filesize
    && lhs.finishedsize == rhs.finishedsize
    && lhs.filectime == rhs.filectime
    && lhs.filemtime == rhs.filemtime
    && lhs.filenum == rhs.filenum;
}


}  // namespace iptux
