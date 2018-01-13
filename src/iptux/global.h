#ifndef IPTUX_GLOBAL_H
#define IPTUX_GLOBAL_H

#include "iptux/CoreThread.h"
#include "iptux/MainWindow.h"
#include "iptux/ProgramData.h"
#include "iptux/SoundSystem.h"
#include "iptux/LogSystem.h"

namespace iptux {

extern ProgramData* g_progdt;
extern CoreThread* g_cthrd;
extern MainWindow* g_mwin;
extern SoundSystem* g_sndsys;
extern LogSystem* g_lgsys;

}

#endif

