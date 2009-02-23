//
// C++ Implementation: Log
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Log.h"
#include "Control.h"
#include "Pal.h"
#include "support.h"
#include "baling.h"
#include "utils.h"
#include "udt.h"

#define LOG_START_HEADER "====================================="
#define LOG_END_HEADER   "-------------------------------------"

 Log::Log():communicate(NULL), system(NULL)
{
}

Log::~Log()
{
	fclose(communicate);
	fclose(system);
}

void Log::InitSelf()
{
	const gchar *env;
	char path[MAX_PATHBUF];

	env = g_get_user_config_dir();
	snprintf(path, MAX_PATHBUF, "%s" LOG_PATH "/communicate.log", env);
	communicate = Fopen(path, "a");
	snprintf(path, MAX_PATHBUF, "%s" LOG_PATH "/system.log", env);
	system = Fopen(path, "a");
}

void Log::CommunicateLog(pointer data, const char *fmt, ...)
{
	extern Control ctr;
	gchar *msg, *ptr;
	va_list ap;
	Pal *pal;

	if (!FLAG_ISSET(ctr.flags, 2))
		return;
	if (data) {
		pal = (Pal *) data;
		ptr = getformattime(_("Nickname: %s\tUser: %s\tHost: %s"),
					    pal->NameQuote(), pal->UserQuote(),
					    pal->HostQuote());
	} else
		ptr = getformattime(_("Me"));
	va_start(ap, fmt);
	msg = g_strdup_vprintf(fmt, ap);
	va_end(ap);
	fprintf(communicate, "%s\n%s%s\n%s\n\n", LOG_START_HEADER,
					    ptr, msg, LOG_END_HEADER);
	g_free(ptr), g_free(msg);
}

void Log::SystemLog(const char *fmt, ...)
{
	extern Control ctr;
	gchar *msg, *ptr;
	va_list ap;

	if (!FLAG_ISSET(ctr.flags, 2))
		return;
	ptr = getformattime(_("Name: %s\tHost: %s"), g_get_real_name(),
					    g_get_host_name());
	va_start(ap, fmt);
	msg = g_strdup_vprintf(fmt, ap);
	va_end(ap);
	fprintf(system, "%s\n%s%s\n%s\n\n", LOG_START_HEADER, ptr,
					    msg, LOG_END_HEADER);
	g_free(ptr), g_free(msg);
}
