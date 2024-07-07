#ifndef IPTUX_PROGRAMDATACORE_H
#define IPTUX_PROGRAMDATACORE_H

#include <memory>
#include <mutex>

#include "iptux-core/IptuxConfig.h"
#include "iptux-core/Models.h"

namespace iptux {

/* flags
// 消息(:7);当有消息时自动打开聊天窗口
// 图标(:6);程序启动后只显示托盘图标而不显示面板
// 传输(:5);当有文件传输时自动打开文件传输窗口
// enter(:4);使用Enter键发送消息
// 历史(:3);关闭好友对话框后自动清空聊天历史
// 日志(:2);开启日志记录功能
// 黑名单(:1);不允许删除的好友再出现
// 共享(:0);好友请求本人的共享文件时需要得到确认
*//* sndfgs
// 传输(:2);文件传输完成后需要播放提示音
// 消息(:1);有消息到来后需要播放提示音
// 声音(:0);是否需要提示音
*/
class ProgramData {
 public:
  explicit ProgramData(std::shared_ptr<IptuxConfig> config);
  virtual ~ProgramData();

  ProgramData(const ProgramData&) = delete;
  ProgramData& operator=(const ProgramData&) = delete;

  std::shared_ptr<IptuxConfig> getConfig();

  /** Sync ProgramData to ConfigFile */
  void WriteProgData();

  const std::vector<NetSegment>& getNetSegments() const;
  void setNetSegments(std::vector<NetSegment>&& netSegments);

  const std::vector<FileInfo>& GetSharedFileInfos() const {
    return sharedFileInfos;
  }
  std::vector<FileInfo>& GetSharedFileInfos() { return sharedFileInfos; }
  void AddShareFileInfo(FileInfo fileInfo);
  void ClearShareFileInfos();
  FileInfo* GetShareFileInfo(uint32_t fileId);
  FileInfo* GetShareFileInfo(uint32_t packetn, uint32_t filenum);

  std::string FindNetSegDescription(in_addr ipv4) const;
  void Lock();
  void Unlock();

  const std::string& GetPasswd() const { return passwd; }
  void SetPasswd(const std::string& val) { passwd = val; }
  int getSendMessageRetryInUs() const { return send_message_retry_in_us; }

  uint16_t port() const { return port_; }
  bool IsAutoOpenChatDialog() const;
  bool IsAutoHidePanelAfterLogin() const;
  bool IsAutoOpenFileTrans() const;
  bool IsEnterSendMessage() const;
  bool IsAutoCleanChatHistory() const;
  bool IsSaveChatHistory() const;
  bool IsUsingBlacklist() const;
  bool IsFilterFileShareRequest() const;
  void set_port(uint16_t port, bool is_init = false);
  void setOpenChat(bool value) { open_chat = value; }
  void setHideStartup(bool value) { hide_startup = value; }
  void setOpenTransmission(bool value) { open_transmission = value; }
  void setUseEnterKey(bool value) { use_enter_key = value; }
  void setClearupHistory(bool value) { clearup_history = value; }
  void setRecordLog(bool value) { record_log = value; }
  void setOpenBlacklist(bool value) { open_blacklist = value; }
  void setProofShared(bool value) { proof_shared = value; }

  bool need_restart() const { return need_restart_; }

  /**
   * @brief Set the Using Blacklist object
   *
   * @param value
   * @return ProgramData& self
   */
  ProgramData& SetUsingBlacklist(bool value);

  std::string nickname;  // 昵称 *
  std::string mygroup;   // 所属群组 *
  std::string myicon;    // 个人头像 *
  std::string path;      // 存档路径 *
  std::string sign;      // 个性签名 *

  std::string codeset;  // 候选编码 *
  std::string encode;   // 默认通信编码 *
  char* palicon;        // 默认头像 *
  char* font;           // 面板字体 *

  struct timeval timestamp;      // 程序数据时间戳
  int send_message_retry_in_us;  // sleep time(in microsecond) when send message
                                 // failed

 private:
  uint16_t port_ = 2425;
  std::vector<NetSegment> netseg;  // 需要通知登录的IP段
  std::shared_ptr<IptuxConfig> config;
  std::mutex mutex;  // 锁
  std::string passwd;
  std::vector<FileInfo> sharedFileInfos;
  uint8_t open_chat : 1;
  uint8_t hide_startup : 1;
  uint8_t open_transmission : 1;
  uint8_t use_enter_key : 1;
  uint8_t clearup_history : 1;
  uint8_t record_log : 1;
  uint8_t open_blacklist : 1;
  uint8_t proof_shared : 1;
  uint8_t need_restart_ : 1;

 private:
  void InitSublayer();
  void ReadProgData();

  void WriteNetSegment();
  void ReadNetSegment();
};
}  // namespace iptux

#endif  // IPTUX_PROGRAMDATACORE_H
