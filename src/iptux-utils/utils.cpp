#include "config.h"
#include "utils.h"

#include <cerrno>

#include <sys/stat.h>

#include <glib/gi18n.h>

#include "iptux-core/output.h"

using namespace std;

namespace iptux {
namespace utils {

int64_t fileOrDirectorySize(const string& fileOrDirName) {
  // 由于系统中存在使用此方法读取文件的大小的调用，因此需要判断文件dir_name是文件还是目录
  struct stat st;
  int result = stat(fileOrDirName.c_str(), &st);
  if (result != 0) {
    // Fail to determine file type, return 0 (判断文件类型失败，直接返回 0)
    LOG_WARN(_("stat file \"%s\" failed: %s"), fileOrDirName.c_str(), strerror(errno));
    return 0;
  }
  if (S_ISREG(st.st_mode)) {
    return st.st_size;
  }
  if (!S_ISDIR(st.st_mode)) {
    LOG_WARN(_("path %s is not file or directory: st_mode(%x)"), fileOrDirName.c_str(),
             st.st_mode);
    return 0;
  }
  // 到了这里就一定是目录了
  DIR *dir = opendir(fileOrDirName.c_str());
  if (dir == NULL) {
    LOG_WARN(_("opendir on \"%s\" failed: %s"), fileOrDirName.c_str(), strerror(errno));
    return 0;
  }

  struct dirent *dirt;
  int64_t sumsize = 0;
  while ((dirt = readdir(dir))) {
    if (strcmp(dirt->d_name, ".") == 0) {
      continue;
    }
    if (strcmp(dirt->d_name, "..") == 0) {
      continue;
    }
    string tpath = fileOrDirName + "/" + dirt->d_name;
    struct stat st;
    if (stat(tpath.c_str(), &st) == -1) {
      continue;
    }
    if (S_ISDIR(st.st_mode)) {
      sumsize += fileOrDirectorySize(tpath);
    } else if (S_ISREG(st.st_mode)) {
      sumsize += st.st_size;
    } else {
      // ignore other files
    }
  }
  return sumsize;
}

}
}
