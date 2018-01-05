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
#include "config.h"

#include <string>

#include <libintl.h>

#include "ProgramData.h"
#include "CoreThread.h"
#include "StatusIcon.h"
#include "MainWindow.h"
#include "LogSystem.h"
#include "SoundSystem.h"
#include "support.h"

using namespace std;

ProgramData* g_progdt;
CoreThread* g_cthrd;
MainWindow* g_mwin;
SoundSystem* g_sndsys;
LogSystem* g_lgsys;

string getConfigPath() {
        const char* res1 =  g_build_path("/",
                g_getenv("HOME"),
                ".iptux",
                "config.json",
                NULL
                );
        string res2(res1);
        g_free(gpointer(res1));
        return res2;
}

int main(int argc, char *argv[])
{
  string configPath = getConfigPath();
  IptuxConfig config(configPath);
  ProgramData progdt(config);
  MainWindow mwin(config, progdt);
  CoreThread cthrd(config);
  StatusIcon sicon(config, mwin);
  LogSystem lgsys;
  SoundSystem sndsys;

  g_progdt = &progdt;
  g_cthrd = &cthrd;
  g_mwin = &mwin;
  g_sndsys = &sndsys;
  g_lgsys = &lgsys;


  mwin.SetStatusIcon(&sicon);

        setlocale(LC_ALL, "");
        bindtextdomain(GETTEXT_PACKAGE, __LOCALE_PATH);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
        textdomain(GETTEXT_PACKAGE);

        analysis_parameter(argc, argv);

        gdk_threads_init();
        gdk_threads_enter();
        gtk_init(&argc, &argv);

        iptux_init();
        sicon.CreateStatusIcon();
        mwin.CreateWindow();
        cthrd.CoreThreadEntry();

        gtk_main();
        gdk_threads_leave();
        return 0;
}
