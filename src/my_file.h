//
// C++ Interface: my_file
//
// Description:虚拟文件系统
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MY_FILE_H
#define MY_FILE_H

#include "udt.h"

class my_file {
 public:
	my_file(bool fg);	//r false,w true
	~my_file();

	void chdir(const char *dir);
	int open(const char *filename, int flags, ...);
	int stat(const char *filename, struct stat64 *st);
	uint64_t ftw(const char *dir);
	DIR *opendir();
 private:
	static int fn(const char *file, const struct stat64 *sb, int flag);
	static uint64_t sumsize;

	char path[MAX_PATHBUF];
	bool flag;
};

#endif
