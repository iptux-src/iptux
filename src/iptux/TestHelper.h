#ifndef IPTUX_TESTHELPER_H
#define IPTUX_TESTHELPER_H

#include <memory>

#include "Application.h"
#include "iptux-core/IptuxConfig.h"
#include <gtk/gtk.h>

namespace iptux {

Application* CreateApplication(IptuxConfig::Ptr config = nullptr);
void DestroyApplication(Application* app);

}  // namespace iptux

#endif
