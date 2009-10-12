/***************************************************************************
 *   Copyright (C) 2009 by Jally   *
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
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include "utils.h"
#include "../sys.h"

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "");
	bindtextdomain(GETTEXT_PACKAGE, __LOCALE_PATH);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	check_privileged();

	remove_file(__DESKTOP_PATH "/iptux.desktop");
	remove_file(__LOGO_PATH "/hicolor/64x64/apps/ip-tux.png");
	remove_file(__LOGO_PATH "/hicolor/64x64/apps/i-tux.png");
	remove_file(__LOGO_PATH "/hicolor/48x48/apps/ip-tux.png");
	remove_file(__LOGO_PATH "/hicolor/48x48/apps/i-tux.png");
	remove_file(__LOGO_PATH "/hicolor/32x32/apps/ip-tux.png");
	remove_file(__LOGO_PATH "/hicolor/32x32/apps/i-tux.png");
	remove_file(__LOGO_PATH "/hicolor/24x24/apps/ip-tux.png");
	remove_file(__LOGO_PATH "/hicolor/24x24/apps/i-tux.png");
	remove_file(__LOGO_PATH "/hicolor/22x22/apps/ip-tux.png");
	remove_file(__LOGO_PATH "/hicolor/22x22/apps/i-tux.png");
	remove_file(__LOGO_PATH "/hicolor/16x16/apps/ip-tux.png");
	remove_file(__LOGO_PATH "/hicolor/16x16/apps/i-tux.png");

	remove_file(__EXEC_PATH "/iptux");
	remove_file(__EXEC_PATH "/ihateiptux");

	remove_folder(__DATA_PATH "/iptux");
	remove_file(__LOCALE_PATH "/en_GB/LC_MESSAGES/iptux.mo");
	remove_file(__LOCALE_PATH "/zh_CN/LC_MESSAGES/iptux.mo");
	remove_file(__LOCALE_PATH "/zh_TW/LC_MESSAGES/iptux.mo");
	
	return 0;
}
