/***************************************************************************
 *   Copyright (C) 2008 by Jally   *
 *   jallyx@163.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "common.h"
#include "MainWindow.h"
#include "StatusIcon.h"
#include "CoreThread.h"
#include "support.h"
#include "utils.h"

int main(int argc, char *argv[])
{
	StatusIcon icon;
	MainWindow window;

	bindtextdomain(GETTEXT_PACKAGE, __LOCALE_DIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
	if (!g_thread_supported())
		g_thread_init(NULL);
	gdk_threads_init();
	gdk_threads_enter();
	gtk_init(&argc, &argv);

	iptux_init();
	mwp = &window;
	icon.CreateStatusIcon();
	thread_create(ThreadFunc(CoreThread::CoreThreadEntry), NULL, false);
	window.CreateWindow();

	gtk_main();
	gdk_threads_leave();
	return 0;
}
