#ifndef IPTUX_GLOBAL_H
#define IPTUX_GLOBAL_H

#include "iptux/UiCoreThread.h"
#include "iptux/MainWindow.h"
#include "iptux/UiProgramData.h"
#include "iptux/SoundSystem.h"

namespace iptux {

extern UiProgramData* g_progdt;
extern UiCoreThread* g_cthrd;
extern MainWindow* g_mwin;
extern SoundSystem* g_sndsys;

}  // namespace iptux

#endif
