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
#include <string>

#include <libintl.h>

#include "iptux/config.h"
#include "iptux/ipmsg.h"
#include "iptux/ProgramData.h"
#include "iptux/CoreThread.h"
#include "iptux/StatusIcon.h"
#include "iptux/MainWindow.h"
#include "iptux/LogSystem.h"
#include "iptux/SoundSystem.h"
#include "iptux/support.h"
#include "iptux/deplib.h"

using namespace std;
using namespace iptux;

namespace iptux {
ProgramData* g_progdt;
CoreThread* g_cthrd;
MainWindow* g_mwin;
SoundSystem* g_sndsys;
LogSystem* g_lgsys;
}

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

static gboolean version = FALSE;
static gchar* configFilename = nullptr;

static GOptionEntry entries[] =
{
  { "version", 'v', 0, G_OPTION_ARG_NONE, &version, "Output version information and exit", NULL },
  { "config", 'c', 0, G_OPTION_ARG_FILENAME, &configFilename, "Specify config path", "CONFIG_PATH"},
  { NULL }
};

int main (int argc, char *argv[]) {
  setlocale(LC_ALL, "");
  bindtextdomain(GETTEXT_PACKAGE, __LOCALE_PATH);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);

  GError *error = NULL;
  GOptionContext *context;

  context = g_option_context_new(_("- A software for sharing in LAN"));
  g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
  g_option_context_add_group (context, gtk_get_option_group (TRUE));
  if (!g_option_context_parse (context, &argc, &argv, &error)) {
    g_print(_("option parsing failed: %s\n"), error->message);
    exit (1);
  }
  if(version) {
    printf("iptux: " VERSION "\n");
    exit(0);
  }
  string configPath = configFilename ? configFilename : getConfigPath();
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

        gdk_threads_init();
        gdk_threads_enter();
        gtk_init(&argc, &argv);

        int port = config.GetInt("port", IPTUX_DEFAULT_PORT);
        iptux_init(port);
        sicon.CreateStatusIcon();
        mwin.CreateWindow();
        cthrd.CoreThreadEntry();

        gtk_main();
        gdk_threads_leave();
        return 0;
}
