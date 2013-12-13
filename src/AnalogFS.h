//
// C++ Interface: AnalogFS
//
// Description:
// 虚拟文件系统，接口函数使用方法尽量与库函数相同
// 意义在于使用此类操作文件可保证多线程安全
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ANALOGFS_H
#define ANALOGFS_H

#include "mess.h"

class AnalogFS {
public:
        AnalogFS();
        ~AnalogFS();

        int chdir(const char *dir);
        int open(const char *fn, int flags);
        int open(const char *fn, int flags, mode_t mode);
        int stat(const char *fn, struct stat64 *st);
        int mkdir(const char *dir, mode_t mode);
        int64_t ftwsize(const char *dir);
        DIR *opendir(const char *dir);
private:
        int mergepath(char tpath[], const char *npath);

        char path[MAX_PATHLEN]; //当前工作路径
public:
        inline char* cwd() {
                        return path;
                }
};

#endif
