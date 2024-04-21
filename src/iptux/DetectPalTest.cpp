#include "Application.h"
#include "gtest/gtest.h"

#include "iptux/DetectPal.h"
#include "iptux/TestHelper.h"

using namespace std;
using namespace iptux;

TEST(DetectPal, Constructor) {
  Application* app = CreateApplication();
  DetectPal pal(app, nullptr);
  DestroyApplication(app);
}
