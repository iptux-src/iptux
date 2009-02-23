//
// C++ Implementation: utils
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "utils.h"
#include "../sys.h"
#define MAX_PATHBUF 1024

void check_privileged()
{
	struct passwd *pw;

	pw = getpwnam("root");
	endpwent();
	if (pw && (pw->pw_uid != geteuid()))
		errx(1, _("The user is not privileged!\n"));
}

void remove_folder(const char *path)
{
	char buf[MAX_PATHBUF];
	struct stat st;
	struct dirent *dirt;
	DIR *dir;

	if (!(dir = opendir(path))) {
		warnx(_("act: open directory '%s',warning: %s\n"),
				      path, strerror(errno));
		return;
	}

	while (dirt = readdir(dir)) {
		if ((strcmp(dirt->d_name, ".") == 0)
			   || (strcmp(dirt->d_name, "..") == 0))
			continue;
		snprintf(buf, MAX_PATHBUF, "%s/%s", path, dirt->d_name);
		if (stat(buf, &st) == -1) {
			warnx(_("act: look up file's status '%s',warning: %s\n"),
						      buf, strerror(errno));
			continue;
		}

		if (S_ISLNK(st.st_mode) || S_ISREG(st.st_mode)) {
			if (unlink(buf) == -1)
				warnx(_("act: unlink file '%s',warning: %s\n"),
						      buf, strerror(errno));
		}
		else if (S_ISDIR(st.st_mode)) {
			remove_folder(buf);
		}
	}
	closedir(dir);

	if (rmdir(path) == -1)
		warnx(_("act: rmdir directory '%s',warning: %s\n"),
				      buf, strerror(errno));
}

void remove_file(const char *path)
{
	if (unlink(path) == -1)
		warnx(_("act: unlink file '%s',warning: %s\n"),
				      path, strerror(errno));
}
