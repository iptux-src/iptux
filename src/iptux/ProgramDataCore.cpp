#include "ProgramDataCore.h"

#include <unistd.h>
#include <sys/time.h>

#include "iptux/config.h"
#include "iptux/deplib.h"
#include "iptux/ipmsg.h"
#include "iptux/utils.h"

using namespace std;

namespace iptux {

/**
 * 类构造函数.
 */
ProgramDataCore::ProgramDataCore(IptuxConfig &config)
    : palicon(NULL),
      font(NULL),
      transtip(NULL),
      msgtip(NULL),
      volume(1.0),
      sndfgs(uint8_t(~0)),
      urlregex(NULL),
      config(config),
      flags(0) {
  gettimeofday(&timestamp, NULL);
  pthread_mutex_init(&mutex, NULL);
  InitSublayer();
}

/**
 * 类析构函数.
 */
ProgramDataCore::~ProgramDataCore() {
  g_free(palicon);
  g_free(font);

  g_free(msgtip);
  g_free(transtip);

  if (urlregex) g_regex_unref(urlregex);
  pthread_mutex_destroy(&mutex);
}

IptuxConfig& ProgramDataCore::getConfig() {
  return config;
}

/**
 * 初始化相关类成员数据.
 */
void ProgramDataCore::InitSublayer() {
  ReadProgData();
  CreateRegex();
}

/**
 * 写出程序数据.
 */
void ProgramDataCore::WriteProgData() {
  gettimeofday(&timestamp, NULL);  //更新时间戳
  config.SetString("nick_name", nickname);
  config.SetString("belong_group", mygroup);
  config.SetString("my_icon", myicon);
  config.SetString("archive_path", path);
  config.SetString("personal_sign", sign);

  config.SetString("candidacy_encode", codeset);
  config.SetString("preference_encode", encode);
  config.SetString("pal_icon", palicon);
  config.SetString("panel_font", font);

  config.SetBool("open_chat", FLAG_ISSET(flags, 7));
  config.SetBool("hide_startup", FLAG_ISSET(flags, 6));
  config.SetBool("open_transmission", FLAG_ISSET(flags, 5));
  config.SetBool("use_enter_key", FLAG_ISSET(flags, 4));
  config.SetBool("clearup_history", FLAG_ISSET(flags, 3));
  config.SetBool("record_log", FLAG_ISSET(flags, 2));
  config.SetBool("open_blacklist", FLAG_ISSET(flags, 1));
  config.SetBool("proof_shared", FLAG_ISSET(flags, 0));

  config.SetString("trans_tip", transtip);
  config.SetString("msg_tip", msgtip);
  config.SetDouble("volume_degree", volume);

  config.SetBool("transnd_support", FLAG_ISSET(sndfgs, 2));
  config.SetBool("msgsnd_support", FLAG_ISSET(sndfgs, 1));
  config.SetBool("sound_support", FLAG_ISSET(sndfgs, 0));

  config.Save();

  WriteNetSegment();
}

/**
 * 深拷贝一份网段数据.
 * @return 网段数据
 */
vector<NetSegment> ProgramDataCore::CopyNetSegment() {
  return netseg;
}

/**
 * 查询(ipv4)所在网段的描述串.
 * @param ipv4 ipv4
 * @return 描述串
 */
char *ProgramDataCore::FindNetSegDescription(in_addr_t ipv4) {
  in_addr_t startip, endip;
  NetSegment *pns;
  char *description;

  ipv4 = ntohl(ipv4);
  description = NULL;
  for(int i = 0; i < netseg.size(); ++i) {
    pns = &netseg[i];
    inet_pton(AF_INET, pns->startip.c_str(), &startip);
    startip = ntohl(startip);
    inet_pton(AF_INET, pns->endip.c_str(), &endip);
    endip = ntohl(endip);
    ipv4_order(&startip, &endip);
    if (ipv4 >= startip && ipv4 <= endip) {
      description = g_strdup(pns->description.c_str());
      break;
    }
  }

  return description;
}

/**
 * 读取程序数据.
 */
void ProgramDataCore::ReadProgData() {
  nickname = config.GetString("nick_name", g_get_user_name());
  mygroup = config.GetString("belong_group");
  myicon = config.GetString("my_icon", "icon-tux.png");
  path = config.GetString("archive_path", g_get_home_dir());
  sign = config.GetString("personal_sign");

  codeset = config.GetString("candidacy_encode", "gb18030,utf-16");
  encode = config.GetString("preference_encode", "utf-8");
  palicon = g_strdup(config.GetString("pal_icon", "icon-qq.png").c_str());
  font = g_strdup(config.GetString("panel_font", "Sans Serif 10").c_str());

  FLAG_SET(flags, 7, config.GetBool("open_chat"));
  FLAG_SET(flags, 6, config.GetBool("hide_startup"));
  FLAG_SET(flags, 5, config.GetBool("open_transmission"));
  FLAG_SET(flags, 4, config.GetBool("use_enter_key"));
  FLAG_SET(flags, 3, config.GetBool("clearup_history"));
  FLAG_SET(flags, 2, config.GetBool("record_log"));
  FLAG_SET(flags, 1, config.GetBool("open_blacklist"));
  FLAG_SET(flags, 0, config.GetBool("proof_shared"));

  msgtip =
      g_strdup(config.GetString("msg_tip", __SOUND_PATH "/msg.ogg").c_str());
  transtip = g_strdup(
      config.GetString("trans_tip", __SOUND_PATH "/trans.ogg").c_str());
  volume = config.GetDouble("volume_degree");

  FLAG_SET(flags, 2, config.GetBool("transnd_support"));
  FLAG_SET(flags, 1, config.GetBool("msgsnd_support"));
  FLAG_SET(flags, 0, config.GetBool("sound_support"));
}

/**
 * 创建识别URL的正则表达式.
 */
void ProgramDataCore::CreateRegex() {
  urlregex =
      g_regex_new(URL_REGEX, GRegexCompileFlags(0), GRegexMatchFlags(0), NULL);
}


/**
 * 写出网段数据.
 */
void ProgramDataCore::WriteNetSegment() {
  vector<Json::Value> jsons;

  pthread_mutex_lock(&mutex);
  for(int i = 0; i < netseg.size(); ++i) {
    jsons.push_back(netseg[i].ToJsonValue());
  }
  pthread_mutex_unlock(&mutex);
  config.SetVector("scan_net_segement", jsons);
}

/**
 * 读取网段数据.
 * @param client GConfClient
 */
void ProgramDataCore::ReadNetSegment() {
  vector<Json::Value> values = config.GetVector("scan_net_segment");
  for (size_t i = 0; i < values.size(); ++i) {
    netseg.push_back(NetSegment::fromJsonValue(values[i]));
  }
}

void ProgramDataCore::Lock() { pthread_mutex_lock(&mutex); }

void ProgramDataCore::Unlock() { pthread_mutex_unlock(&mutex); }

bool ProgramDataCore::IsAutoOpenCharDialog() const { return FLAG_ISSET(flags, 7); }

bool ProgramDataCore::IsAutoHidePanelAfterLogin() const {
  return FLAG_ISSET(flags, 6);
}

bool ProgramDataCore::IsAutoOpenFileTrans() const { return FLAG_ISSET(flags, 5); }
bool ProgramDataCore::IsEnterSendMessage() const { return FLAG_ISSET(flags, 4); }
bool ProgramDataCore::IsAutoCleanChatHistory() const {
  return FLAG_ISSET(flags, 3);
}
bool ProgramDataCore::IsSaveChatHistory() const { return FLAG_ISSET(flags, 2); }
bool ProgramDataCore::IsUsingBlacklist() const { return FLAG_ISSET(flags, 1); }
bool ProgramDataCore::IsFilterFileShareRequest() const {
  return FLAG_ISSET(flags, 0);
}

void ProgramDataCore::SetFlag(int idx, bool flag) {
  if (flag) {
    FLAG_SET(flags, idx);
  } else {
    FLAG_CLR(flags, idx);
  }
}

}  // namespace iptux

