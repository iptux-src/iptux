#include "gtest/gtest.h"

#include "iptux/Application.h"
#include "iptux/DataSettings.h"
#include "iptux/TestHelper.h"

using namespace std;
using namespace iptux;

TEST(DataSettings, Constructor) {
  Application* app = CreateApplication();
  DataSettings::ResetDataEntry(app, nullptr, false);
  DestroyApplication(app);
}
