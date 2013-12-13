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
#include "AnalogFS.h"
#include "output.h"
#include "utils.h"

/**
 * 类构造函数.
 */
AnalogFS::AnalogFS()
{
        if (!::getcwd(path, MAX_PATHLEN))
                strcpy(path, "/");
}

/**
 * 类析构函数.
 */
AnalogFS::~AnalogFS()
{
}

/**
 * 更改当前工作目录.
 * @param dir 目录路径
 * @return 成功与否
 */
int AnalogFS::chdir(const char *dir)
{
        size_t len;
        char *ptr;

        if (strcmp(dir, ".") == 0)
                return 0;

        if (*dir != '/') {
                if (strcmp(dir, "..") == 0) {
                        ptr = strrchr(path, '/');
                        if (ptr != path)
                                *ptr = '\0';
                } else {
                        len = strlen(path);
                        ptr = (char *)(*(path + 1) != '\0' ? "/" : "");
                        snprintf(path + len, MAX_PATHLEN - len, "%s%s", ptr, dir);
                }
        } else
                snprintf(path, MAX_PATHLEN, "%s", dir);

        return 0;
}

int AnalogFS::open(const char *fn, int flags) {
        return open(fn, flags, 0);
}


/**
 * 打开文件.
 * @param fn 文件路径
 * @param flags as in open()
 * @param ... as in open()
 * @return 文件描述符
 */
int AnalogFS::open(const char *fn, int flags, mode_t mode)
{
        char tpath[MAX_PATHLEN];
        char *tfn;
        int fd;

        strcpy(tpath, path);
        mergepath(tpath, fn);
        if ((flags & O_ACCMODE) == O_WRONLY) {
                tfn = assert_filename_inexist(tpath);
                if ((fd = ::open(tfn, flags, mode)) == -1)
                        pwarning(_("Open() file \"%s\" failed, %s"), tfn,
                                                         strerror(errno));
                g_free(tfn);
        } else {
                if ((fd = ::open(tpath, flags, mode)) == -1)
                        pwarning(_("Open() file \"%s\" failed, %s"), tpath,
                                                         strerror(errno));
        }
        return fd;
}

/**
 * 获取文件状态.
 * @param fn 文件路径
 * @param st a stat64 struct
 * @return 成功与否
 */
int AnalogFS::stat(const char *fn, struct stat64 *st)
{
        char tpath[MAX_PATHLEN];
        int result;

        strcpy(tpath, path);
        mergepath(tpath, fn);
        if ((result = ::stat64(tpath, st)) != 0)
                pwarning(_("Stat64() file \"%s\" failed, %s"), tpath, strerror(errno));

        return result;
}

/**
 * 创建新的目录.
 * @param dir 目录路径
 * @param mode as in mkdir()
 * @return 成功与否
 */
int AnalogFS::mkdir(const char *dir, mode_t mode)
{
        char tpath[MAX_PATHLEN];
        int result;

        strcpy(tpath, path);
        mergepath(tpath, dir);
        if (::access(tpath, F_OK) == 0)
                return 0;
        if ((result = ::mkdir(tpath, mode)) != 0)
                pwarning(_("Mkdir() directory \"%s\" failed, %s"), tpath,
                                                         strerror(errno));

        return result;
}

/**
 * 获取目录总大小.
 * @param dir_name 目录路径
 * @return 目录大小
 */
int64_t AnalogFS::ftwsize(const char *dir_name)
{
        DIR* dir = opendir(dir_name);
        if(dir == NULL) {
                pwarning(_("opendir on \"%s\" failed: %s"), dir_name, strerror(errno));
                return 0;
        }

        struct dirent *dirt;
        int64_t sumsize = 0;
        while(dirt = readdir(dir)) {
                if(strcmp(dirt->d_name, ".") == 0) {
                        continue;
                }
                if(strcmp(dirt->d_name, "..") == 0) {
                        continue;
                }
                char tpath[MAX_PATHLEN];
                strcpy(tpath, dir_name);
                mergepath(tpath, dirt->d_name);
                struct stat64 st;
                if(stat64(tpath, &st) == -1) {
                        continue;
                }
                if(S_ISDIR(st.st_mode)) {
                        sumsize += ftwsize(tpath);
                } else if(S_ISREG(st.st_mode)) {
                        sumsize += st.st_size;
                } else {
                        // ignore other files
                }

        }
        return sumsize;
}


/**
 * 打开目录.
 * @param dir 目录路径
 * @return DIR
 */
DIR *AnalogFS::opendir(const char *dir)
{
        char tpath[MAX_PATHLEN];
        DIR *dirs;

        strcpy(tpath, path);
        mergepath(tpath, dir);
        if (!(dirs = ::opendir(tpath)))
                pwarning(_("Opendir() directory \"%s\" failed, %s"), tpath,
                                                         strerror(errno));

        return dirs;
}

/**
 * 合并路径.
 * @param tpath[] 基路径
 * @param npath 需要被合并的路径
 * @return 成功与否
 */
int AnalogFS::mergepath(char tpath[], const char *npath)
{
        size_t len;
        char *ptr;

        if (strcmp(npath, ".") == 0)
                return 0;

        if (*npath != '/') {
                if (strcmp(npath, "..") == 0) {
                        ptr = strrchr(tpath, '/');
                        if (ptr != tpath)
                                *ptr = '\0';
                } else {
                        len = strlen(tpath);
                        ptr = (char *)(*(tpath + 1) != '\0' ? "/" : "");
                        snprintf(tpath + len, MAX_PATHLEN - len, "%s%s", ptr, npath);
                }
        } else
                snprintf(tpath, MAX_PATHLEN, "%s", npath);

        return 0;
}
