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

#include "iptux-core/internal/ipmsg.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"
#include <cstring>
#include <fcntl.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

namespace iptux {

static bool mergepath(char tpath[], int len, const char* npath);

/**
 * 类构造函数.
 */
AnalogFS::AnalogFS() {
  gchar* current_dir = g_get_current_dir();
  if (current_dir) {
    g_strlcpy(path, current_dir, MAX_PATHLEN);
    g_free(current_dir);
  } else {
    strcpy(path, "/");
  }
  LOG_INFO("AnalogFS initialized, cwd: %s", path);
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
  char* new_path;

  if (g_path_is_absolute(dir)) {
    new_path = g_strdup(dir);
  } else {
    new_path = g_build_filename(this->path, dir, NULL);
  }

  if (!new_path) {
    LOG_WARN("Failed to build new path from \"%s\" and \"%s\"", this->path,
             dir);
    return -1;
  }
  if (strlen(new_path) >= MAX_PATHLEN) {
    LOG_WARN("New path \"%s\" exceeds maximum length of %d", new_path,
             MAX_PATHLEN);
    g_free(new_path);
    return -1;
  }
  strcpy(this->path, new_path);
  g_free(new_path);
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
  if (!mergepath(tpath, MAX_PATHLEN, fn)) {
    LOG_WARN("Merge path failed, \"%s\" + \"%s\"", path, fn);
    return -1;
  }
  if ((flags & O_ACCMODE) == O_WRONLY) {
    auto tfn = assert_filename_inexist(tpath);
    if ((fd = ::open(tfn.c_str(), flags, mode)) == -1) {
      LOG_WARN("Open() file \"%s\" failed, %s", tfn.c_str(), strerror(errno));
    }
  } else {
    if ((fd = ::open(tpath, flags, mode)) == -1) {
      LOG_WARN("Open() file \"%s\" failed, %s", tpath, strerror(errno));
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
  mergepath(tpath, MAX_PATHLEN, fn);
  if ((result = ::stat(tpath, st)) != 0) {
    LOG_WARN(_("Stat64() file \"%s\" failed, %s"), tpath, strerror(errno));
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
  mergepath(tpath, MAX_PATHLEN, dir);
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
  if (!mergepath(tpath, MAX_PATHLEN, dir)) {
    LOG_WARN("Merge path failed, \"%s\" + \"%s\"", path, dir);
    return nullptr;
  }
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
bool mergepath(char tpath[], int len, const char* npath) {
  gchar* full_path;

  if (g_path_is_absolute(npath)) {
    full_path = g_strdup(npath);
  } else {
    full_path = g_build_filename(tpath, npath, NULL);
  }

  if (!full_path) {
    return false;
  }

  if ((int)strlen(full_path) >= len) {
    g_free(full_path);
    return false;
  }

  strcpy(tpath, full_path);
  g_free(full_path);
  return true;
}

}  // namespace iptux
