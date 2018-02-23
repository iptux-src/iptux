#ifndef IPTUX_GLOBAL_H
#define IPTUX_GLOBAL_H

#include "iptux/UiCoreThread.h"
#include "iptux/LogSystem.h"
#include "iptux/MainWindow.h"
#include "iptux/ProgramData.h"
#include "iptux/SoundSystem.h"

namespace iptux {

extern ProgramData* g_progdt;
extern UiCoreThread* g_cthrd;
extern MainWindow* g_mwin;
extern SoundSystem* g_sndsys;
extern LogSystem* g_lgsys;

}  // namespace iptux

#endif
