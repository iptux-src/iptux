#include "config.h"
#include "TestHelper.h"

using namespace std;

namespace iptux {

shared_ptr<IptuxConfig> newTestIptuxConfig() {
  char templateFname[] = "/tmp/iptuxXXXXXX.json";
  char* tmpfile = mktemp(templateFname);
  string fname(tmpfile);
  return shared_ptr<IptuxConfig>(new IptuxConfig(fname));
}

}

