//
// C++ Interface: mess
//
// Description:
// 很杂乱的一些数据基本结构.
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_MODELS_H
#define IPTUX_MODELS_H

#include <arpa/inet.h>
#include <memory>
#include <netinet/in.h>
#include <string>

#include <json/json.h>

enum IptuxMsgSrcType {
  IPTUX_MSG_SRC_PAL,   ///< 好友
  IPTUX_MSG_SRC_SELF,  ///< 自身
  IPTUX_MSG_SRC_ERROR  ///< 错误
};

namespace iptux {

/**
 * 消息来源类型.
 */

/**
 * 消息内容类型.
 */
enum class MessageContentType {
  STRING,  ///< 字符串
  PICTURE  ///< 图片
};

static const MessageContentType MESSAGE_CONTENT_TYPE_STRING =
    MessageContentType::STRING;
static const MessageContentType MESSAGE_CONTENT_TYPE_PICTURE =
    MessageContentType::PICTURE;

/**
 * 群组所属类型
 */
typedef enum {
  GROUP_BELONG_TYPE_REGULAR,   ///< 常规
  GROUP_BELONG_TYPE_SEGMENT,   ///< 分段
  GROUP_BELONG_TYPE_GROUP,     ///< 分组
  GROUP_BELONG_TYPE_BROADCAST  ///< 广播
} GroupBelongType;

class PalKey {
 public:
  PalKey(in_addr ipv4, int port);

  bool operator==(const PalKey& rhs) const;

  in_addr GetIpv4() const { return ipv4; }
  std::string GetIpv4String() const;
  int GetPort() const { return port; }
  std::string ToString() const;

 private:
  in_addr ipv4;
  int port;
};

/**
 * 好友信息.
 * flags位含义: \n
 * 黑名单(:3);此IP地址被列入黑名单(deprecated) \n
 * 更改(:2);好友的信息被用户手工修改，程序不应再更改好友的信息 \n
 * 在线(:1);好友依旧在线 \n
 * 兼容(:0);完全兼容iptux，程序将采用扩展协议与好友通信 \n
 */
class PalInfo {
 public:
  PalInfo(const std::string& ipv4, uint16_t port);
  PalInfo(in_addr ipv4, uint16_t port);
  ~PalInfo();

  PalKey GetKey() const { return PalKey(ipv4(), port_); }

  PalInfo& setName(const std::string& name);
  const std::string& getName() const { return name; }

  PalInfo& setUser(const std::string& user);
  const std::string& getUser() const { return user; }

  PalInfo& setHost(const std::string& host);
  const std::string& getHost() const { return host; }

  PalInfo& setVersion(const std::string& version);
  const std::string& getVersion() const { return version; }

  PalInfo& setEncode(const std::string& encode);
  const std::string& getEncode() const { return encode; }

  PalInfo& setGroup(const std::string& group);
  const std::string& getGroup() const { return group; }

  const std::string& icon_file() const { return icon_file_; }
  PalInfo& set_icon_file(const std::string& icon_file) {
    icon_file_ = icon_file;
    return *this;
  }
  PalInfo& set_icon_file(const std::string& icon_file, const std::string& def) {
    if (icon_file.empty())
      icon_file_ = def;
    else
      icon_file_ = icon_file;
    return *this;
  }

  std::string toString() const;
  in_addr ipv4() const { return ipv4_; }
  uint16_t port() const { return port_; }

  char* segdes;       ///< 所在网段描述
  char* photo;        ///< 形象照片
  char* sign;         ///< 个性签名
  uint32_t packetn;   ///< 已接受最大的包编号
  uint32_t rpacketn;  ///< 需要接受检查的包编号

  bool isCompatible() const;
  bool isOnline() const;
  bool isChanged() const;
  bool isInBlacklist() const;

  PalInfo& setCompatible(bool value);
  PalInfo& setOnline(bool value);
  PalInfo& setChanged(bool value);
  PalInfo& setInBlacklist(bool value);

 private:
  in_addr ipv4_;           ///< 好友IP
  uint16_t port_;          ///< 好友端口
  std::string icon_file_;  ///< 好友头像 *
  std::string user;
  std::string name;
  std::string host;
  std::string version;  ///< 版本串 *
  std::string encode;   ///< 好友编码 *
  std::string group;    ///< 所在群组
  uint8_t compatible : 1;
  uint8_t online : 1;
  uint8_t changed : 1;
  uint8_t in_blacklist : 1;
};

/// pointer to PalInfo
using PPalInfo = std::shared_ptr<PalInfo>;

/// const pointer to PalInfo
using CPPalInfo = std::shared_ptr<const PalInfo>;

enum class FileAttr { UNKNOWN, REGULAR, DIRECTORY };

constexpr bool FileAttrIsValid(FileAttr attr) {
  return attr == FileAttr::REGULAR || attr == FileAttr::DIRECTORY;
}

/**
 * 文件信息.
 */
class FileInfo {
 public:
  FileInfo();
  ~FileInfo();

  FileInfo(const FileInfo& fileInfo);
  FileInfo& operator=(const FileInfo& fileInfo);

  bool operator==(const FileInfo& rhs) const;
  bool isExist() const;
  void ensureFilesizeFilled();

  uint32_t fileid;       ///< 唯一标识
  uint32_t packetn;      ///< 包编号
  FileAttr fileattr;     ///< 文件属性
  int64_t filesize;      ///< 文件大小
  int64_t finishedsize;  ///< 已完成大小
  CPPalInfo fileown;     ///< 文件拥有者(来自好友*)
  char* filepath;        ///< 文件路径 *
  uint32_t filectime;    ///<  文件创建时间
  uint32_t filemtime;    ///<  文件最后修改时间
  uint32_t filenum;      ///<  包内编号
};
using PFileInfo = std::shared_ptr<FileInfo>;

/**
 * 碎片数据.
 */
class ChipData {
 public:
  explicit ChipData(const std::string& data);
  ChipData(MessageContentType type, const std::string& data);
  ~ChipData();

  std::string ToString() const;
  std::string getSummary() const;

  MessageContentType type;  ///< 消息内容类型
  std::string data;         ///< 数据串 *

  void SetDeleteFileAfterSent(bool val) { deleteFileAfterSent = val; }
  bool GetDeleteFileAfterSent() const { return deleteFileAfterSent; }

 private:
  bool deleteFileAfterSent{true};
};

/**
 * 消息参数.
 */
class MsgPara {
 public:
  explicit MsgPara(CPPalInfo pal);
  ~MsgPara();

  std::string getSummary() const;

  CPPalInfo getPal() const { return pal; }

  IptuxMsgSrcType stype;         ///< 来源类型
  GroupBelongType btype;         ///< 所属类型
  std::vector<ChipData> dtlist;  ///< 数据链表 *
 private:
  CPPalInfo pal;  ///< 好友数据信息(来自好友*)
};

/**
 * 网段数据.
 */
class NetSegment {
 public:
  NetSegment(std::string startIp, std::string endIp, std::string description);
  NetSegment();
  ~NetSegment();

  bool ContainIP(in_addr ipv4) const;
  /**
   * @brief return the ip count in this segment
   *
   */
  uint64_t Count() const;
  std::string NthIp(uint64_t i) const;

  std::string startip;      ///< IP起始地址 *
  std::string endip;        ///< IP终止地址 *
  std::string description;  ///< 此IP段描述

  Json::Value ToJsonValue() const;
  static NetSegment fromJsonValue(Json::Value& value);
};

}  // namespace iptux

#endif
