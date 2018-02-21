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
#include <unistd.h>

#include "iptux/deplib.h"
#include "iptux/ipmsg.h"
#include "iptux/utils.h"

#define LOG_START_HEADER "====================================="
#define LOG_END_HEADER "-------------------------------------"

namespace iptux {

LogSystem::LogSystem(const ProgramDataCore& programDataCore)
    : programDataCore(programDataCore),
      fdc(-1),
      fds(-1) {}

LogSystem::~LogSystem() {
  close(fdc);
  close(fds);
}

void LogSystem::InitSublayer() {
  const gchar *env;
  char path[MAX_PATHLEN];

  env = g_get_user_config_dir();
  snprintf(path, MAX_PATHLEN, "%s" LOG_PATH "/communicate.log", env);
  fdc = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
  snprintf(path, MAX_PATHLEN, "%s" LOG_PATH "/system.log", env);
  fds = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
}

void LogSystem::CommunicateLog(MsgPara *msgpara, const char *fmt, ...) {
  gchar *log, *msg, *ptr;
  va_list ap;

  if (!programDataCore.IsSaveChatHistory()) {
    return;
  }

  PalInfo *pal = msgpara->pal;

  if (msgpara->stype == MessageSourceType::PAL)
    ptr = getformattime(TRUE, _("Recevied-From: Nickname:%s User:%s Host:%s"),
                        pal->name, pal->user, pal->host);
  else if (msgpara->stype == MessageSourceType::SELF) {
    if (msgpara->pal)
      ptr = getformattime(TRUE, _("Send-To: Nickname:%s User:%s Host:%s"),
                          pal->name, pal->user, pal->host);
    else
      ptr = getformattime(TRUE, _("Send-Broadcast"));
  } else
    return;

  va_start(ap, fmt);
  msg = g_strdup_vprintf(fmt, ap);
  va_end(ap);
  log = g_strdup_printf("%s\n%s\n%s\n%s\n\n", LOG_START_HEADER, ptr, msg,
                        LOG_END_HEADER);
  write(fdc, log, strlen(log));
  g_free(log);
  g_free(ptr);
  g_free(msg);
}

void LogSystem::SystemLog(const char *fmt, ...) {
  gchar *log, *msg, *ptr;
  va_list ap;

  if (!programDataCore.IsSaveChatHistory()) {
    return;
  }
  ptr = getformattime(TRUE, _("User:%s Host:%s"), g_get_user_name(),
                      g_get_host_name());
  va_start(ap, fmt);
  msg = g_strdup_vprintf(fmt, ap);
  va_end(ap);
  log = g_strdup_printf("%s\n%s\n%s\n%s\n\n", LOG_START_HEADER, ptr, msg,
                        LOG_END_HEADER);
  g_free(ptr);
  g_free(msg);

  write(fds, log, strlen(log));
  g_free(log);
}

}  // namespace iptux
