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
    : palicon(NULL), font(NULL), config(config), need_restart_(0) {
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

  config->SetBool("open_chat", open_chat);
  config->SetBool("hide_startup", hide_startup);
  config->SetBool("open_transmission", open_transmission);
  config->SetBool("use_enter_key", use_enter_key);
  config->SetBool("clearup_history", clearup_history);
  config->SetBool("record_log", record_log);
  config->SetBool("open_blacklist", open_blacklist);
  config->SetBool("proof_shared", proof_shared);
  config->SetBool("hide_taskbar_when_main_window_iconified",
                  hide_taskbar_when_main_window_iconified_);
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

void ProgramData::set_port(uint16_t port, bool is_init) {
  if (port == port_)
    return;

  uint16_t old_port = port_;
  port_ = port;
  if (port_ < 1024 || port_ > 65535) {
    LOG_WARN("Invalid port number: %d, use default port: %d", port_,
             IPTUX_DEFAULT_PORT);
    port_ = IPTUX_DEFAULT_PORT;
  }
  if (!is_init && old_port != port_)
    need_restart_ = true;
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

  set_port(config->GetInt("port", IPTUX_DEFAULT_PORT), true);
  codeset = config->GetString("candidacy_encode", "gb18030,utf-16");
  encode = config->GetString("preference_encode", "utf-8");
  palicon = g_strdup(config->GetString("pal_icon", "icon-qq.png").c_str());
  font = g_strdup(config->GetString("panel_font", "Sans Serif 10").c_str());

  open_chat = config->GetBool("open_chat");
  hide_startup = config->GetBool("hide_startup");
  open_transmission = config->GetBool("open_transmission");
  use_enter_key = config->GetBool("use_enter_key");
  clearup_history = config->GetBool("clearup_history");
  record_log = config->GetBool("record_log", true);
  open_blacklist = config->GetBool("open_blacklist");
  proof_shared = config->GetBool("proof_shared");
  hide_taskbar_when_main_window_iconified_ =
      config->GetBool("hide_taskbar_when_main_window_iconified");

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

bool ProgramData::IsAutoOpenChatDialog() const {
  return open_chat;
}

bool ProgramData::IsAutoHidePanelAfterLogin() const {
  return hide_startup;
}

bool ProgramData::IsAutoOpenFileTrans() const {
  return open_transmission;
}
bool ProgramData::IsEnterSendMessage() const {
  return use_enter_key;
}
bool ProgramData::IsAutoCleanChatHistory() const {
  return clearup_history;
}
bool ProgramData::IsSaveChatHistory() const {
  return record_log;
}
bool ProgramData::IsUsingBlacklist() const {
  return open_blacklist;
}
bool ProgramData::IsFilterFileShareRequest() const {
  return proof_shared;
}

bool ProgramData::isHideTaskbarWhenMainWindowIconified() const {
#if HAVE_APPINDICATOR
  return hide_taskbar_when_main_window_iconified_;
#else
  return false;
#endif
}

ProgramData& ProgramData::SetUsingBlacklist(bool value) {
  open_blacklist = value;
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
