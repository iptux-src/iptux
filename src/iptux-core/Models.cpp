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
#include <cstring>
#include <netinet/in.h>
#include <sstream>
#include <unistd.h>

#include "iptux-core/internal/AnalogFS.h"
#include "iptux-core/internal/ipmsg.h"
#include "iptux-utils/utils.h"

using namespace std;

namespace iptux {

PalInfo::PalInfo(in_addr ipv4, uint16_t port)
    : segdes(NULL),
      photo(NULL),
      sign(NULL),
      packetn(0),
      rpacketn(0),
      key_(ipv4, port),
      compatible(0),
      online(0),
      changed(0),
      in_blacklist(0) {}

PalInfo::PalInfo(const string& ipv4, uint16_t port)
    : segdes(NULL),
      photo(NULL),
      sign(NULL),
      packetn(0),
      rpacketn(0),
      key_(inAddrFromString(ipv4), port),
      compatible(0),
      online(0),
      changed(0),
      in_blacklist(0) {}

PalInfo::~PalInfo() {
  g_free(segdes);
  g_free(photo);
  g_free(sign);
}

bool PalInfo::isCompatible() const {
  return compatible;
}

bool PalInfo::isOnline() const {
  return online;
}

bool PalInfo::isChanged() const {
  return changed;
}

PalInfo& PalInfo::setCompatible(bool value) {
  this->compatible = value;
  return *this;
}

PalInfo& PalInfo::setOnline(bool value) {
  this->online = value;
  return *this;
}

PalInfo& PalInfo::setChanged(bool value) {
  this->changed = value;
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
      "photo=%s,sign=%s,iconfile=%s,encode=%s,packetn=%d,rpacketn=%d,"
      "compatible=%d,online=%d,changed=%d,in_blacklist=%d)",
      inAddrToString(ipv4()).c_str(), name.c_str(), segdes, version.c_str(),
      user.c_str(), host.c_str(), group.c_str(), photo ? photo : "(NULL)",
      sign ? sign : "(NULL)", icon_file_.c_str(), encode.c_str(), int(packetn),
      int(rpacketn), compatible, online, changed, in_blacklist);
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

bool NetSegment::ContainIP(GInetAddress* ipv4) const {
  GInetAddress* start = g_inet_address_new_from_string(startip.c_str());
  GInetAddress* end = g_inet_address_new_from_string(endip.c_str());

  gsize size = g_inet_address_get_native_size(ipv4);
  const guint8* ip_bytes = g_inet_address_to_bytes(ipv4);
  const guint8* start_bytes = g_inet_address_to_bytes(start);
  const guint8* end_bytes = g_inet_address_to_bytes(end);

  int cmp_start = memcmp(ip_bytes, start_bytes, size);
  int cmp_end = memcmp(ip_bytes, end_bytes, size);

  g_object_unref(start);
  g_object_unref(end);

  return cmp_start >= 0 && cmp_end <= 0;
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

PalKey::PalKey(in_addr ipv4, int port) {
  GInetAddress* inet_addr = g_inet_address_new_from_bytes(
      reinterpret_cast<const guint8*>(&ipv4), G_SOCKET_FAMILY_IPV4);
  address_ = g_inet_socket_address_new(inet_addr, port);
  g_object_unref(inet_addr);
}

PalKey::PalKey(GSocketAddress* address) {
  address_ = G_SOCKET_ADDRESS(g_object_ref(address));
}

PalKey::~PalKey() {
  if (address_) {
    g_object_unref(address_);
  }
}

PalKey::PalKey(const PalKey& other) {
  address_ = G_SOCKET_ADDRESS(g_object_ref(other.address_));
}

PalKey& PalKey::operator=(const PalKey& other) {
  if (this != &other) {
    if (address_) {
      g_object_unref(address_);
    }
    address_ = G_SOCKET_ADDRESS(g_object_ref(other.address_));
  }
  return *this;
}

PalKey::PalKey(PalKey&& other) noexcept : address_(other.address_) {
  other.address_ = nullptr;
}

PalKey& PalKey::operator=(PalKey&& other) noexcept {
  if (this != &other) {
    if (address_) {
      g_object_unref(address_);
    }
    address_ = other.address_;
    other.address_ = nullptr;
  }
  return *this;
}

in_addr PalKey::GetIpv4() const {
  GInetAddress* inet_addr = g_inet_socket_address_get_address(
      G_INET_SOCKET_ADDRESS(address_));
  
  in_addr result;
  gsize size = g_inet_address_get_native_size(inet_addr);
  if (size == sizeof(in_addr)) {
    memcpy(&result, g_inet_address_to_bytes(inet_addr), sizeof(in_addr));
  } else {
    // Should not happen for IPv4
    memset(&result, 0, sizeof(in_addr));
  }
  return result;
}

string PalKey::GetIpv4String() const {
  GInetAddress* inet_addr = g_inet_socket_address_get_address(
      G_INET_SOCKET_ADDRESS(address_));
  gchar* str = g_inet_address_to_string(inet_addr);
  string result(str);
  g_free(str);
  return result;
}

int PalKey::GetPort() const {
  return g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(address_));
}

bool PalKey::operator==(const PalKey& rhs) const {
  GInetSocketAddress* this_addr = G_INET_SOCKET_ADDRESS(this->address_);
  GInetSocketAddress* other_addr = G_INET_SOCKET_ADDRESS(rhs.address_);
  
  // Compare ports
  if (g_inet_socket_address_get_port(this_addr) != 
      g_inet_socket_address_get_port(other_addr)) {
    return false;
  }
  
  // Compare addresses
  GInetAddress* this_inet = g_inet_socket_address_get_address(this_addr);
  GInetAddress* other_inet = g_inet_socket_address_get_address(other_addr);
  
  return g_inet_address_equal(this_inet, other_inet);
}

string PalKey::ToString() const {
  return stringFormat("%s:%d", GetIpv4String().c_str(), GetPort());
}

GSocketAddress* PalKey::GetSocketAddress() const {
  return G_SOCKET_ADDRESS(g_object_ref(address_));
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
