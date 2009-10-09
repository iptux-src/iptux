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

/**
 * 打开文件.
 * @param fn 文件路径
 * @param flags as in open()
 * @param ... as in open()
 * @return 文件描述符
 */
int AnalogFS::open(const char *fn, int flags, ...)
{
	char tpath[MAX_PATHLEN];
	va_list ap;
	char *tfn;
	int fd;

	strcpy(tpath, path);
	mergepath(tpath, fn);
	va_start(ap, flags);
	if ((flags & O_ACCMODE) == O_WRONLY) {
		tfn = assert_filename_inexist(tpath);
		if ((fd = ::open(tfn, flags, va_arg(ap, mode_t))) == -1)
			pwarning(_("Open() file \"%s\" failed, %s"), tfn,
							 strerror(errno));
		g_free(tfn);
	} else {
		if ((fd = ::open(tpath, flags, va_arg(ap, mode_t))) == -1)
			pwarning(_("Open() file \"%s\" failed, %s"), tpath,
							 strerror(errno));
	}
	va_end(ap);

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
 * @param dir 目录路径
 * @return 目录大小
 */
int64_t AnalogFS::ftwsize(const char *dir)
{
	struct stat64 st;
	struct dirent *dirt;
	DIR *dirs;
	char tpath[MAX_PATHLEN];
	int64_t sumsize;

	LIST_HEAD(listhead, entry) head;
	struct entry {
		LIST_ENTRY(entry) entries;
		void *data;
	} *item;
	LIST_INIT(&head);

	strcpy(tpath, path);
	mergepath(tpath, dir);
	sumsize = 0;
	dirs = NULL;
	goto mark;
	while ( (item = head.lh_first)) {
		dirs = (DIR *)item->data;
		LIST_REMOVE(item, entries);
		g_free(item);
		while (dirs && (dirt = ::readdir(dirs))) {
			if (strcmp(dirt->d_name, ".") == 0
				 || strcmp(dirt->d_name, "..") == 0)
				continue;
			mergepath(tpath, dirt->d_name);
mark:			if ((::stat64(tpath, &st) == -1)
				 || !(S_ISREG(st.st_mode) || S_ISDIR(st.st_mode))) {
				mergepath(tpath, "..");
				continue;
			}
			if (S_ISDIR(st.st_mode)) {
				item = (struct entry *)g_malloc(sizeof(struct entry));
				item->data = dirs;
				LIST_INSERT_HEAD(&head, item, entries);
				dirs = ::opendir(tpath);
			} else {
				sumsize += st.st_size;
				mergepath(tpath, "..");
			}
		}
		if (dirs) {
			::closedir(dirs);
			mergepath(tpath, "..");
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
