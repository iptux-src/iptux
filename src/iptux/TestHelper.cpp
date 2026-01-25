#include "config.h"

#include "Application.h"
#include "UiCoreThread.h"
#include "gtest/gtest.h"
#include "iptux-core/TestHelper.h"
#include "iptux-utils/output.h"
#include "iptux/TestHelper.h"

namespace iptux {

Application* CreateApplication() {
  gtk_init(nullptr, nullptr);
  auto config = newTestIptuxConfig();
  Application* app = new Application(config);
  app->setTestMode(true);
  app->set_enable_app_indicator(false);
  app->startup();
  // g_application_register(G_APPLICATION(app->getApp()), nullptr, nullptr);
  // auto i = g_application_get_is_registered(G_APPLICATION(app->getApp()));
  // EXPECT_TRUE(i);
  app->getCoreThread()->setIgnoreTcpBindFailed(true);
  app->activate();
  if (app->isActivated() == false) {
    LOG_ERROR("Application activate failed");
    DestroyApplication(app);
    return nullptr;
  }
  return app;
}

void DestroyApplication(Application* app) {
  g_application_quit(G_APPLICATION(app->getApp()));
}
}  // namespace iptux
