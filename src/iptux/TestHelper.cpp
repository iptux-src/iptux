#include "config.h"
#include "TestHelper.h"

using namespace std;

namespace iptux {

shared_ptr<IptuxConfig> newTestIptuxConfig() {
  char* uuid = g_uuid_string_random();
  char* fname = g_strdup_printf("/tmp/iptux%s.json", uuid);
  auto res = shared_ptr<IptuxConfig>(new IptuxConfig(fname));
  g_free(fname);
  g_free(uuid);
  return res;
}

}

