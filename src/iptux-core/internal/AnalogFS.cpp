//
// C++ Implementation: AnalogFS
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "config.h"
#include "AnalogFS.h"

#include <cstring>
#include <fcntl.h>
#include <glib/gstdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "iptux-core/internal/ipmsg.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"

using namespace std;

namespace iptux {

static int mergepath(char tpath[], const char* npath);

/**
 * 类构造函数.
 */
AnalogFS::AnalogFS() {
  if (!::getcwd(path, MAX_PATHLEN)) {
    strcpy(path, "/");
  }
}

/**
 * 类析构函数.
 */
AnalogFS::~AnalogFS() {}

/**
 * 更改当前工作目录.
 * @param dir 目录路径
 * @return 成功与否
 */
int AnalogFS::chdir(const char* dir) {
  size_t len;
  char* ptr;

  if (strcmp(dir, ".") == 0)
    return 0;

  if (*dir != '/') {
    if (strcmp(dir, "..") == 0) {
      ptr = strrchr(path, '/');
      if (ptr != path)
        *ptr = '\0';
    } else {
      len = strlen(path);
      ptr = (char*)(*(path + 1) != '\0' ? "/" : "");
      snprintf(path + len, MAX_PATHLEN - len, "%s%s", ptr, dir);
    }
  } else
    snprintf(path, MAX_PATHLEN, "%s", dir);

  return 0;
}

int AnalogFS::open(const char* fn, int flags) {
  return open(fn, flags, 0);
}

/**
 * 打开文件.
 * @param fn 文件路径
 * @param flags as in open()
 * @param ... as in open()
 * @return 文件描述符
 */
int AnalogFS::open(const char* fn, int flags, mode_t mode) {
  char tpath[MAX_PATHLEN];
  int fd;

  strcpy(tpath, path);
  mergepath(tpath, fn);
  if ((flags & O_ACCMODE) == O_WRONLY) {
    auto tfn = assert_filename_inexist(tpath);
    if ((fd = ::open(tfn.c_str(), flags, mode)) == -1) {
      pwarning(_("Open() file \"%s\" failed, %s"), tfn.c_str(),
               strerror(errno));
    }
  } else {
    if ((fd = ::open(tpath, flags, mode)) == -1) {
      pwarning(_("Open() file \"%s\" failed, %s"), tpath, strerror(errno));
    }
  }
  return fd;
}

/**
 * 获取文件状态.
 * @param fn 文件路径
 * @param st a stat64 struct
 * @return 成功与否
 */
int AnalogFS::stat(const char* fn, struct ::stat* st) {
  char tpath[MAX_PATHLEN];
  int result;

  strcpy(tpath, path);
  mergepath(tpath, fn);
  if ((result = ::stat(tpath, st)) != 0) {
    pwarning(_("Stat64() file \"%s\" failed, %s"), tpath, strerror(errno));
  }

  return result;
}

/**
 * 创建新的目录.
 * @param dir 目录路径
 * @param mode as in mkdir()
 * @return 成功与否
 */
int AnalogFS::makeDir(const char* dir, mode_t mode) {
  char tpath[MAX_PATHLEN];
  int result;

  strcpy(tpath, path);
  mergepath(tpath, dir);
  if (::access(tpath, F_OK) == 0)
    return 0;
  if ((result = g_mkdir(tpath, mode)) != 0) {
    pwarning(_("Mkdir() directory \"%s\" failed, %s"), tpath, strerror(errno));
  }
  return result;
}

/**
 * 获取目录总大小.
 * @param dir_name 目录路径
 * @return 目录大小
 */
int64_t AnalogFS::ftwsize(const char* dir_name) {
  return utils::fileOrDirectorySize(dir_name);
}

/**
 * 打开目录.
 * @param dir 目录路径
 * @return DIR
 */
DIR* AnalogFS::opendir(const char* dir) {
  char tpath[MAX_PATHLEN];
  DIR* dirs;

  strcpy(tpath, path);
  mergepath(tpath, dir);
  if (!(dirs = ::opendir(tpath))) {
    pwarning(_("Opendir() directory \"%s\" failed, %s"), tpath,
             strerror(errno));
  }
  return dirs;
}

/**
 * 合并路径.
 * @param tpath[] 基路径
 * @param npath 需要被合并的路径
 * @return 成功与否
 */
int mergepath(char tpath[], const char* npath) {
  size_t len;
  char* ptr;

  if (strcmp(npath, ".") == 0)
    return 0;

  if (*npath != '/') {
    if (strcmp(npath, "..") == 0) {
      ptr = strrchr(tpath, '/');
      if (ptr != tpath)
        *ptr = '\0';
    } else {
      len = strlen(tpath);
      ptr = (char*)(*(tpath + 1) != '\0' ? "/" : "");
      snprintf(tpath + len, MAX_PATHLEN - len, "%s%s", ptr, npath);
    }
  } else
    snprintf(tpath, MAX_PATHLEN, "%s", npath);

  return 0;
}

}  // namespace iptux
