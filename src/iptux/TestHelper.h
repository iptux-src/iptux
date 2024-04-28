#ifndef IPTUX_TESTHELPER_H
#define IPTUX_TESTHELPER_H

#include "Application.h"
#include <gtk/gtk.h>

namespace iptux {

Application* CreateApplication();
void DestroyApplication(Application* app);

}  // namespace iptux

#endif
