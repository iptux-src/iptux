#include "iptux_crash_utils.h"

#include <string>

#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>

using namespace std;

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

    int status;
    char* demangled = abi::__cxa_demangle(symname, NULL, 0, &status);
    if (status == 0 && demangled)
      symname = demangled;

    off64_t offset = 0;
    if (dlinfo.dli_saddr) {
      offset = (off64_t)(trace[i]) - (off64_t)(dlinfo.dli_saddr);
    } else {
      offset = (off64_t)(trace[i]) - (off64_t)(dlinfo.dli_fbase);
    }

    // fprintf(stderr, "%s(%s+0x%lx)[%p][%p][%p]\n",
    //         dlinfo.dli_fname ? dlinfo.dli_fname : "null",
    //         symname ? symname : "", offset, trace[i], dlinfo.dli_fbase,
    //         dlinfo.dli_saddr);
    fprintf(stderr, "%-3d %-40s %p %s + 0x%lx\n", i,
            dlinfo.dli_fname ? dlinfo.dli_fname : "null", trace[i],
            symname ? symname : "", offset);
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
