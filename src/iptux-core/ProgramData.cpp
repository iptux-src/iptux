#include "config.h"
#include "iptux-core/ProgramData.h"

#include <sys/stat.h>
#include <sys/time.h>

#include "iptux-core/internal/ipmsg.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"

using namespace std;

namespace iptux {

static const char* CONFIG_SHARED_FILE_LIST = "shared_file_list";

/**
 * 类构造函数.
 */
ProgramData::ProgramData(shared_ptr<IptuxConfig> config)
    : palicon(NULL), font(NULL), config(config), flags(0) {
  gettimeofday(&timestamp, NULL);
  InitSublayer();
}

/**
 * 类析构函数.
 */
ProgramData::~ProgramData() {
  g_free(palicon);
  g_free(font);
}

shared_ptr<IptuxConfig> ProgramData::getConfig() {
  return config;
}

/**
 * 初始化相关类成员数据.
 */
void ProgramData::InitSublayer() {
  ReadProgData();
}

/**
 * 写出程序数据.
 */
void ProgramData::WriteProgData() {
  gettimeofday(&timestamp, NULL);  // 更新时间戳
  config->SetString("nick_name", nickname);
  config->SetString("belong_group", mygroup);
  config->SetString("my_icon", myicon);
  config->SetString("archive_path", path);
  config->SetString("personal_sign", sign);

  config->SetInt("port", port_);
  config->SetString("candidacy_encode", codeset);
  config->SetString("preference_encode", encode);
  config->SetString("pal_icon", palicon);
  config->SetString("panel_font", font);

  config->SetBool("open_chat", FLAG_ISSET(flags, 7));
  config->SetBool("hide_startup", FLAG_ISSET(flags, 6));
  config->SetBool("open_transmission", FLAG_ISSET(flags, 5));
  config->SetBool("use_enter_key", FLAG_ISSET(flags, 4));
  config->SetBool("clearup_history", FLAG_ISSET(flags, 3));
  config->SetBool("record_log", FLAG_ISSET(flags, 2));
  config->SetBool("open_blacklist", FLAG_ISSET(flags, 1));
  config->SetBool("proof_shared", FLAG_ISSET(flags, 0));

  config->SetString("access_shared_limit", passwd);
  config->SetInt("send_message_retry_in_us", send_message_retry_in_us);
  WriteNetSegment();

  vector<string> sharedFileList;
  for (const FileInfo& fileInfo : sharedFileInfos) {
    sharedFileList.push_back(fileInfo.filepath);
  }
  config->SetStringList(CONFIG_SHARED_FILE_LIST, sharedFileList);
  config->Save();
}

const std::vector<NetSegment>& ProgramData::getNetSegments() const {
  return netseg;
}

void ProgramData::setNetSegments(std::vector<NetSegment>&& netSegments) {
  netseg = netSegments;
}

void ProgramData::set_port(uint16_t port) {
  port_ = port;
  if (port_ < 1024 || port_ > 65535) {
    LOG_WARN("Invalid port number: %d, use default port: %d", port_,
             IPTUX_DEFAULT_PORT);
    port_ = IPTUX_DEFAULT_PORT;
  }
}

/**
 * 查询(ipv4)所在网段的描述串.
 * @param ipv4 ipv4
 * @return 描述串
 */
string ProgramData::FindNetSegDescription(in_addr ipv4) const {
  for (size_t i = 0; i < netseg.size(); ++i) {
    if (netseg[i].ContainIP(ipv4)) {
      return netseg[i].description;
    }
  }
  return "";
}

/**
 * 读取程序数据.
 */
void ProgramData::ReadProgData() {
  nickname = config->GetString("nick_name", g_get_user_name());
  mygroup = config->GetString("belong_group");
  myicon = config->GetString("my_icon", "icon-tux.png");
  path = config->GetString("archive_path", g_get_home_dir());
  sign = config->GetString("personal_sign");

  set_port(config->GetInt("port", IPTUX_DEFAULT_PORT));
  codeset = config->GetString("candidacy_encode", "gb18030,utf-16");
  encode = config->GetString("preference_encode", "utf-8");
  palicon = g_strdup(config->GetString("pal_icon", "icon-qq.png").c_str());
  font = g_strdup(config->GetString("panel_font", "Sans Serif 10").c_str());

  FLAG_SET(flags, 7, config->GetBool("open_chat"));
  FLAG_SET(flags, 6, config->GetBool("hide_startup"));
  FLAG_SET(flags, 5, config->GetBool("open_transmission"));
  FLAG_SET(flags, 4, config->GetBool("use_enter_key"));
  FLAG_SET(flags, 3, config->GetBool("clearup_history"));
  FLAG_SET(flags, 2, config->GetBool("record_log", true));
  FLAG_SET(flags, 1, config->GetBool("open_blacklist"));
  FLAG_SET(flags, 0, config->GetBool("proof_shared"));

  passwd = config->GetString("access_shared_limit");
  send_message_retry_in_us =
      config->GetInt("send_message_retry_in_us", 1000000);
  if (send_message_retry_in_us <= 0) {
    send_message_retry_in_us = 1000000;
  }

  ReadNetSegment();

  /* 读取共享文件数据 */
  vector<string> sharedFileList =
      config->GetStringList(CONFIG_SHARED_FILE_LIST);

  /* 分析数据并加入文件链表 */
  sharedFileInfos.clear();
  int pbn = 1;
  for (size_t i = 0; i < sharedFileList.size(); ++i) {
    struct stat st;
    if (stat(sharedFileList[i].c_str(), &st) == -1 ||
        !(S_ISREG(st.st_mode) || S_ISDIR(st.st_mode))) {
      continue;
    }
    /* 加入文件信息到链表 */
    FileInfo fileInfo;
    fileInfo.fileid = pbn++;
    fileInfo.fileattr =
        S_ISREG(st.st_mode) ? FileAttr::REGULAR : FileAttr::DIRECTORY;
    fileInfo.filepath = strdup(sharedFileList[i].c_str());
    sharedFileInfos.emplace_back(fileInfo);
  }
}

/**
 * 写出网段数据.
 */
void ProgramData::WriteNetSegment() {
  vector<Json::Value> jsons;
  {
    lock_guard<std::mutex> l(mutex);
    for (size_t i = 0; i < netseg.size(); ++i) {
      jsons.push_back(netseg[i].ToJsonValue());
    }
  }
  config->SetVector("scan_net_segment", jsons);
}

/**
 * 读取网段数据.
 * @param client GConfClient
 */
void ProgramData::ReadNetSegment() {
  vector<Json::Value> values = config->GetVector("scan_net_segment");
  for (size_t i = 0; i < values.size(); ++i) {
    netseg.push_back(NetSegment::fromJsonValue(values[i]));
  }
}

void ProgramData::Lock() {
  mutex.lock();
}

void ProgramData::Unlock() {
  mutex.unlock();
}

bool ProgramData::IsAutoOpenCharDialog() const {
  return FLAG_ISSET(flags, 7);
}

bool ProgramData::IsAutoHidePanelAfterLogin() const {
  return FLAG_ISSET(flags, 6);
}

bool ProgramData::IsAutoOpenFileTrans() const {
  return FLAG_ISSET(flags, 5);
}
bool ProgramData::IsEnterSendMessage() const {
  return FLAG_ISSET(flags, 4);
}
bool ProgramData::IsAutoCleanChatHistory() const {
  return FLAG_ISSET(flags, 3);
}
bool ProgramData::IsSaveChatHistory() const {
  return FLAG_ISSET(flags, 2);
}
bool ProgramData::IsUsingBlacklist() const {
  return FLAG_ISSET(flags, 1);
}
bool ProgramData::IsFilterFileShareRequest() const {
  return FLAG_ISSET(flags, 0);
}

void ProgramData::SetFlag(int idx, bool flag) {
  if (flag) {
    FLAG_SET(flags, idx);
  } else {
    FLAG_CLR(flags, idx);
  }
}

ProgramData& ProgramData::SetUsingBlacklist(bool value) {
  SetFlag(1, value);
  return *this;
}

FileInfo* ProgramData::GetShareFileInfo(uint32_t fileId) {
  for (const FileInfo& fileInfo : sharedFileInfos) {
    if (fileInfo.fileid == fileId) {
      return new FileInfo(fileInfo);
    }
  }
  return nullptr;
}

FileInfo* ProgramData::GetShareFileInfo(uint32_t packetn, uint32_t filenum) {
  for (const FileInfo& fileInfo : sharedFileInfos) {
    if (fileInfo.packetn == packetn && fileInfo.filenum == filenum) {
      return new FileInfo(fileInfo);
    }
  }
  return nullptr;
}

void ProgramData::ClearShareFileInfos() {
  sharedFileInfos.clear();
}

void ProgramData::AddShareFileInfo(FileInfo fileInfo) {
  sharedFileInfos.emplace_back(std::move(fileInfo));
}

}  // namespace iptux
