#include "iptux_crash_utils.h"

#include <cstdint>
#include <string>

#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

using namespace std;

namespace iptux {

#define MAX_DEPTH 99

static void segvHandler(int sig) {
  void* trace[MAX_DEPTH];
  int stack_depth = backtrace(trace, MAX_DEPTH);

  fprintf(stderr, "Error: signal %d:\n", sig);
  for (int i = 0; i < stack_depth; i++) {
    Dl_info dlinfo;
    if (!dladdr(trace[i], &dlinfo)) {
      fprintf(stderr, "[%p]\n", trace[i]);
      continue;
    }

    const char* symname = dlinfo.dli_sname;
    char* demangled = 0;
    if (symname) {
      int status;
      demangled = abi::__cxa_demangle(symname, NULL, 0, &status);
      if (status == 0 && demangled) {
        symname = demangled;
      }
    }

    const char* fname = dlinfo.dli_fname;
    if (fname) {
      const char* p = strrchr(fname, '/');
      if (p) {
        fname = p + 1;
      }
    } else {
      fname = "null";
    }

    off_t offset = 0;
    if (dlinfo.dli_saddr) {
      offset = (off_t)(trace[i]) - (off_t)(dlinfo.dli_saddr);
    } else {
      offset = (off_t)(trace[i]) - (off_t)(dlinfo.dli_fbase);
    }

    fprintf(stderr, "%-3d %-40s %p %s + 0x%jx\n", i, fname, trace[i],
            symname ? symname : "", (uintmax_t)offset);
    if (demangled) {
      free(demangled);
    }
  }
  exit(1);
}

void installCrashHandler() {
  signal(SIGSEGV, segvHandler);
  signal(SIGABRT, segvHandler);
  signal(SIGTRAP, segvHandler);
}
}  // namespace iptux
