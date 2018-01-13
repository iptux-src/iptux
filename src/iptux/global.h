#ifndef IPTUX_GLOBAL_H
#define IPTUX_GLOBAL_H

#include "CoreThread.h"
#include "MainWindow.h"
#include "ProgramData.h"
#include "SoundSystem.h"
#include "LogSystem.h"

extern ProgramData* g_progdt;
extern CoreThread* g_cthrd;
extern MainWindow* g_mwin;
extern SoundSystem* g_sndsys;
extern LogSystem* g_lgsys;

#endif

