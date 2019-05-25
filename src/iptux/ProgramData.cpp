#include "config.h"
#include "ProgramData.h"

#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "iptux/deplib.h"
#include "iptux/ipmsg.h"
#include "iptux/utils.h"

using namespace std;

namespace iptux {

static const char *CONFIG_SHARED_FILE_LIST = "shared_file_list";

/**
 * 类构造函数.
 */
ProgramData::ProgramData(shared_ptr<IptuxConfig> config)
    : palicon(NULL),
      font(NULL),
      transtip(NULL),
      msgtip(NULL),
      volume(1.0),
      sndfgs(uint8_t(~0)),
      urlregex(NULL),
      config(config),
      flags(0)
{
  gettimeofday(&timestamp, NULL);
  pthread_mutex_init(&mutex, NULL);
  InitSublayer();
}

/**
 * 类析构函数.
 */
ProgramData::~ProgramData() {
  g_free(palicon);
  g_free(font);

  g_free(msgtip);
  g_free(transtip);

  if (urlregex) g_regex_unref(urlregex);
  pthread_mutex_destroy(&mutex);
}

shared_ptr<IptuxConfig> ProgramData::getConfig() {
  return config;
}

/**
 * 初始化相关类成员数据.
 */
void ProgramData::InitSublayer() {
  ReadProgData();
  CreateRegex();
}

/**
 * 写出程序数据.
 */
void ProgramData::WriteProgData() {
  gettimeofday(&timestamp, NULL);  //更新时间戳
  config->SetString("nick_name", nickname);
  config->SetString("belong_group", mygroup);
  config->SetString("my_icon", myicon);
  config->SetString("archive_path", path);
  config->SetString("personal_sign", sign);

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

  config->SetString("trans_tip", transtip);
  config->SetString("msg_tip", msgtip);
  config->SetDouble("volume_degree", volume);

  config->SetBool("transnd_support", FLAG_ISSET(sndfgs, 2));
  config->SetBool("msgsnd_support", FLAG_ISSET(sndfgs, 1));
  config->SetBool("sound_support", FLAG_ISSET(sndfgs, 0));
  config->SetString("access_shared_limit", passwd);
  WriteNetSegment();

  vector<string> sharedFileList;
  for(const FileInfo& fileInfo: sharedFileInfos) {
    sharedFileList.push_back(fileInfo.filepath);
  }
  config->SetStringList(CONFIG_SHARED_FILE_LIST, sharedFileList);
  config->Save();
}

/**
 * 深拷贝一份网段数据.
 * @return 网段数据
 */
vector<NetSegment> ProgramData::copyNetSegments() const {
  return netseg;
}

const std::vector<NetSegment>& ProgramData::getNetSegments() const {
  return netseg;
}

void ProgramData::setNetSegments(std::vector<NetSegment>&& netSegments) {
  netseg = netSegments;
}


/**
 * 查询(ipv4)所在网段的描述串.
 * @param ipv4 ipv4
 * @return 描述串
 */
string ProgramData::FindNetSegDescription(in_addr_t ipv4) const {
  for (int i = 0; i < netseg.size(); ++i) {
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

  msgtip =
      g_strdup(config->GetString("msg_tip", __SOUND_PATH "/msg.ogg").c_str());
  transtip = g_strdup(
      config->GetString("trans_tip", __SOUND_PATH "/trans.ogg").c_str());
  volume = config->GetDouble("volume_degree");

  FLAG_SET(sndfgs, 2, config->GetBool("transnd_support", true));
  FLAG_SET(sndfgs, 1, config->GetBool("msgsnd_support", true));
  FLAG_SET(sndfgs, 0, config->GetBool("sound_support", true));

  passwd = config->GetString("access_shared_limit");

  ReadNetSegment();

  /* 读取共享文件数据 */
  vector<string> sharedFileList = config->GetStringList(CONFIG_SHARED_FILE_LIST);

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
    fileInfo.fileattr = S_ISREG(st.st_mode) ? IPMSG_FILE_REGULAR : IPMSG_FILE_DIR;
    fileInfo.filepath = strdup(sharedFileList[i].c_str());
    sharedFileInfos.emplace_back(fileInfo);
  }
}

/**
 * 创建识别URL的正则表达式.
 */
void ProgramData::CreateRegex() {
  urlregex =
      g_regex_new(URL_REGEX, GRegexCompileFlags(0), GRegexMatchFlags(0), NULL);
}


/**
 * 写出网段数据.
 */
void ProgramData::WriteNetSegment() {
  vector<Json::Value> jsons;

  pthread_mutex_lock(&mutex);
  for(int i = 0; i < netseg.size(); ++i) {
    jsons.push_back(netseg[i].ToJsonValue());
  }
  pthread_mutex_unlock(&mutex);
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

void ProgramData::Lock() { pthread_mutex_lock(&mutex); }

void ProgramData::Unlock() { pthread_mutex_unlock(&mutex); }

bool ProgramData::IsAutoOpenCharDialog() const { return FLAG_ISSET(flags, 7); }

bool ProgramData::IsAutoHidePanelAfterLogin() const {
  return FLAG_ISSET(flags, 6);
}

bool ProgramData::IsAutoOpenFileTrans() const { return FLAG_ISSET(flags, 5); }
bool ProgramData::IsEnterSendMessage() const { return FLAG_ISSET(flags, 4); }
bool ProgramData::IsAutoCleanChatHistory() const {
  return FLAG_ISSET(flags, 3);
}
bool ProgramData::IsSaveChatHistory() const { return FLAG_ISSET(flags, 2); }
bool ProgramData::IsUsingBlacklist() const { return FLAG_ISSET(flags, 1); }
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

}  // namespace iptux

