//
// C++ Implementation: my_file
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "my_file.h"
#include "output.h"
#include "baling.h"
#include "support.h"

uint64_t my_file::sumsize = 0;
my_file::my_file(bool fg)
{
	if (!getcwd(path, MAX_PATHBUF))
		path[0] = '\0';
	flag = fg;
}

my_file::~my_file()
{
}

void my_file::chdir(const char *dir)
{
	size_t len;
	char *ptr;

	if (strcmp(dir, ".") == 0)
		return;

	if (strcmp(dir, "..") == 0) {
		ptr = strrchr(path, '/');
		if (ptr && ptr != path)
			*ptr = '\0';
	} else {
		if (*dir != '/') {
			len = strlen(path);
			snprintf(path + len, MAX_PATHBUF - len, "/%s", dir);
		} else
			snprintf(path, MAX_PATHBUF, "%s", dir);
		if (flag && access(path, F_OK) != 0)
			Mkdir(path, 0777);
	}
}

int my_file::open(const char *filename, int flags, ...)
{
	int fd;
	bool tmp;
	char *tpath;
	va_list ap;

	tmp = flag, flag = false;
	chdir(filename);
	va_start(ap, flags);
	if ((flags & O_ACCMODE) == O_WRONLY) {
		tpath = assert_file_inexistent(path);
		fd = Open(tpath, flags, va_arg(ap, mode_t));
		free(tpath);
	} else
		fd = Open(path, flags, va_arg(ap, mode_t));
	va_end(ap);
	chdir("..");
	flag = tmp;

	return fd;
}

int my_file::stat(const char *filename, struct stat64 *st)
{
	int result;
	bool tmp;

	tmp = flag, flag = false;
	chdir(filename);
	result = Stat(path, st);
	chdir("..");
	flag = tmp;

	return result;
}

uint64_t my_file::ftw(const char *dir)
{
	chdir(dir);
	sumsize = 0;
	::ftw64(path, fn, 255);

	return sumsize;
}

DIR *my_file::opendir()
{
	DIR *dir;

	dir =::opendir(path);
	if (!dir)
		pwarning(Fail, _("act: open directory '%s',warning: %s\n"),
			 path, strerror(errno));

	return dir;
}

int my_file::fn(const char *file, const struct stat64 *sb, int flag)
{
	if (flag == FTW_F)
		sumsize += sb->st_size;
	return 0;
}
