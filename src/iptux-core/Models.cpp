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

#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <sstream>
#include <unistd.h>

#include "iptux-core/internal/AnalogFS.h"
#include "iptux-core/internal/ipmsg.h"
#include "iptux-utils/utils.h"

using namespace std;

namespace iptux {

PalInfo::PalInfo()
    : ipv4({0}),
      segdes(NULL),
      photo(NULL),
      sign(NULL),
      iconfile(NULL),
      packetn(0),
      rpacketn(0),
      flags(0) {}

PalInfo::~PalInfo() {
  g_free(segdes);
  g_free(photo);
  g_free(sign);
  g_free(iconfile);
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

PalInfo& PalInfo::setCompatible(bool value) {
  if (value) {
    FLAG_SET(this->flags, 0);
  } else {
    FLAG_CLR(this->flags, 0);
  }
  return *this;
}

PalInfo& PalInfo::setOnline(bool value) {
  if (value) {
    FLAG_SET(this->flags, 1);
  } else {
    FLAG_CLR(this->flags, 1);
  }
  return *this;
}

PalInfo& PalInfo::setChanged(bool value) {
  if (value) {
    FLAG_SET(this->flags, 2);
  } else {
    FLAG_CLR(this->flags, 2);
  }
  return *this;
}

PalInfo& PalInfo::setName(const std::string& name) {
  this->name = utf8MakeValid(name);
  return *this;
}

PalInfo& PalInfo::setUser(const std::string& user) {
  this->user = utf8MakeValid(user);
  return *this;
}

PalInfo& PalInfo::setHost(const std::string& host) {
  this->host = utf8MakeValid(host);
  return *this;
}

PalInfo& PalInfo::setVersion(const std::string& version) {
  this->version = utf8MakeValid(version);
  return *this;
}

PalInfo& PalInfo::setEncode(const std::string& encode) {
  this->encode = utf8MakeValid(encode);
  return *this;
}

PalInfo& PalInfo::setGroup(const std::string& group) {
  this->group = utf8MakeValid(group);
  return *this;
}

string PalInfo::toString() const {
  return stringFormat(
      "PalInfo(IP=%s,name=%s,segdes=%s,version=%s,user=%s,host=%s,group=%s,"
      "photo=%s,sign=%s,iconfile=%s,encode=%s,packetn=%d,rpacketn=%d,flags=%d)",
      inAddrToString(ipv4).c_str(), name.c_str(), segdes, version.c_str(),
      user.c_str(), host.c_str(), group.c_str(), photo ? photo : "(NULL)",
      sign ? sign : "(NULL)", iconfile, encode.c_str(), int(packetn),
      int(rpacketn), int(flags));
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

FileInfo::~FileInfo() {
  g_free(filepath);
}

FileInfo::FileInfo(const FileInfo& f)
    : fileid(f.fileid),
      packetn(f.packetn),
      fileattr(f.fileattr),
      filesize(f.filesize),
      finishedsize(f.finishedsize),
      fileown(f.fileown),
      filectime(f.filectime),
      filemtime(f.filemtime),
      filenum(f.filenum) {
  filepath = g_strdup(f.filepath);
}

bool FileInfo::isExist() const {
  return g_access(filepath, F_OK) != -1;
}

void FileInfo::ensureFilesizeFilled() {
  if (filesize >= 0) {
    return;
  }
  AnalogFS afs;
  filesize = afs.ftwsize(filepath);
}

MsgPara::MsgPara(CPPalInfo pal)
    : stype(MessageSourceType::PAL),
      btype(GROUP_BELONG_TYPE_REGULAR),
      pal(pal) {}

MsgPara::~MsgPara() {}

string MsgPara::getSummary() const {
  if (this->dtlist.empty()) {
    return _("Empty Message");
  }
  return this->dtlist[0].getSummary();
}

ChipData::ChipData(const string& data)
    : type(MessageContentType::STRING), data(data) {}
ChipData::ChipData(MessageContentType type, const string& data)
    : type(type), data(data) {}
ChipData::~ChipData() {}

NetSegment::NetSegment() {}

NetSegment::NetSegment(string startip, string endip, string description)
    : startip(startip), endip(endip), description(description) {}

NetSegment::~NetSegment() {}

uint64_t NetSegment::Count() const {
  uint32_t start = inAddrToUint32(inAddrFromString(startip));
  uint32_t end = inAddrToUint32(inAddrFromString(endip));
  if (start > end) {
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
  return ipv4Compare(inAddrFromString(startip), ipv4) <= 0 &&
         ipv4Compare(ipv4, inAddrFromString(endip)) <= 0;
}

Json::Value NetSegment::ToJsonValue() const {
  Json::Value value;
  value["startip"] = startip;
  value["endip"] = endip;
  value["description"] = description;
  return value;
}

NetSegment NetSegment::fromJsonValue(Json::Value& value) {
  NetSegment res;
  res.startip = value["startip"].asString();
  res.endip = value["endip"].asString();
  res.description = value["description"].asString();
  return res;
}

string ChipData::ToString() const {
  ostringstream oss;
  oss << "ChipData(";
  switch (type) {
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

string ChipData::getSummary() const {
  switch (type) {
    case MessageContentType::STRING:
      return data;
    case MessageContentType::PICTURE:
      return _("Received an image");
    default:
      g_assert_not_reached();
  }
  return "";
}

PalKey::PalKey(in_addr ipv4) : ipv4(ipv4), port(IPTUX_DEFAULT_PORT) {}

PalKey::PalKey(in_addr ipv4, int port) : ipv4(ipv4), port(port) {}

bool PalKey::operator==(const PalKey& rhs) const {
  return ipv4Equal(this->ipv4, rhs.ipv4) && this->port == rhs.port;
}

string PalKey::ToString() const {
  return stringFormat("%s:%d", inAddrToString(ipv4).c_str(), port);
}

bool FileInfo::operator==(const FileInfo& rhs) const {
  const FileInfo& lhs = *this;
  return lhs.fileid == rhs.fileid && lhs.packetn == rhs.packetn &&
         lhs.fileattr == rhs.fileattr && lhs.filesize == rhs.filesize &&
         lhs.finishedsize == rhs.finishedsize &&
         lhs.filectime == rhs.filectime && lhs.filemtime == rhs.filemtime &&
         lhs.filenum == rhs.filenum;
}

}  // namespace iptux
