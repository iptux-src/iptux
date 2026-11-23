//
// C++ Implementation: LogSystem
//
// Description:
// 实时写入日志信息
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "config.h"
#include "LogSystem.h"

#include <fcntl.h>
#include <glib/gi18n.h>
#include <glog/logging.h>
#include <unistd.h>

#include "iptux-core/Const.h"
#include "iptux-utils/utils.h"

#define LOG_START_HEADER "====================================="
#define LOG_END_HEADER "-------------------------------------"

using namespace std;

namespace iptux {

LogSystem::LogSystem(shared_ptr<const ProgramData> programData)
    : programData(programData), fdc(-1), fds(-1) {
  CHECK_NOTNULL(programData.get());
  InitSublayer();
}

LogSystem::~LogSystem() {
  close(fdc);
  close(fds);
}

void LogSystem::InitSublayer() {
  fdc = open(getChatLogPath().c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
  fds = open(getSystemLogPath().c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
}

void LogSystem::communicateLog(const MsgPara* msgpara, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  communicateLogv(msgpara, fmt, args);
  va_end(args);
}

void LogSystem::systemLog(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  systemLogv(fmt, args);
  va_end(args);
}

void LogSystem::communicateLogv(const MsgPara* msgpara,
                                const char* fmt,
                                va_list ap) {
  gchar *log, *msg, *ptr;

  if (!programData->IsSaveChatHistory()) {
    return;
  }

  auto pal = msgpara->getPal();

  if (msgpara->stype == MessageSourceType::PAL)
    ptr = getformattime(TRUE, _("Recevied-From: Nickname:%s User:%s Host:%s"),
                        pal->getName().c_str(), pal->getUser().c_str(),
                        pal->getHost().c_str());
  else if (msgpara->stype == MessageSourceType::SELF) {
    if (msgpara->getPal())
      ptr = getformattime(TRUE, _("Send-To: Nickname:%s User:%s Host:%s"),
                          pal->getName().c_str(), pal->getUser().c_str(),
                          pal->getHost().c_str());
    else
      ptr = getformattime(TRUE, _("Send-Broadcast"));
  } else
    return;

  msg = g_strdup_vprintf(fmt, ap);
  log = g_strdup_printf("%s\n%s\n%s\n%s\n\n", LOG_START_HEADER, ptr, msg,
                        LOG_END_HEADER);
  write(fdc, log, strlen(log));
  g_free(log);
  g_free(ptr);
  g_free(msg);
}

void LogSystem::systemLogv(const char* fmt, va_list ap) {
  gchar *log, *msg, *ptr;

  if (!programData->IsSaveChatHistory()) {
    return;
  }
  ptr = getformattime(TRUE, _("User:%s Host:%s"), g_get_user_name(),
                      g_get_host_name());
  msg = g_strdup_vprintf(fmt, ap);
  log = g_strdup_printf("%s\n%s\n%s\n%s\n\n", LOG_START_HEADER, ptr, msg,
                        LOG_END_HEADER);
  g_free(ptr);
  g_free(msg);

  write(fds, log, strlen(log));
  g_free(log);
}

string LogSystem::getChatLogPath() const {
  auto env = g_get_user_config_dir();
  return stringFormat("%s" LOG_PATH "/communicate.log", env);
}

string LogSystem::getSystemLogPath() const {
  auto env = g_get_user_config_dir();
  return stringFormat("%s" LOG_PATH "/system.log", env);
}

}  // namespace iptux
