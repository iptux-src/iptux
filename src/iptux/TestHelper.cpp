#include "config.h"
#include "TestHelper.h"

#include <glib.h>

using namespace std;

namespace iptux {

shared_ptr<IptuxConfig> newTestIptuxConfig() {
  char* fname = g_strdup_printf("/tmp/iptux%d.json", g_random_int());
  auto res = shared_ptr<IptuxConfig>(new IptuxConfig(fname));
  g_free(fname);
  return res;
}

}

