#include "gtest/gtest.h"

#include "iptux/Application.h"
#include "iptux/TestHelper.h"

using namespace std;
using namespace iptux;

TEST(Application, Constructor) {
  Application* app = CreateApplication();
  DestroyApplication(app);
}
