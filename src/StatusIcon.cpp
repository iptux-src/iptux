//
// C++ Implementation: StatusIcon
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "StatusIcon.h"
#include "Transport.h"
#include "IptuxSetting.h"
#include "ShareFile.h"
#include "DialogGroup.h"
#include "DetectPal.h"
#include "UdpData.h"
#include "DialogPeer.h"
#include "output.h"
#include "support.h"
#include "utils.h"

 StatusIcon::StatusIcon():status_icon(NULL)
{
}

StatusIcon::~StatusIcon()
{
	g_object_unref(status_icon);
}

void StatusIcon::CreateStatusIcon()
{
	extern struct interactive inter;
	GdkPixbuf *pixbuf;
	GdkScreen *screen;

	pixbuf = gdk_pixbuf_new_from_file_at_size(__LOGO_DIR "/ip-tux.png",
						  20, 20, NULL);
	if (!pixbuf) {
		pop_error("\n%s \"" __LOGO_DIR "/ip-tux.png\" %s",
			  _("The notify icon"), _("is lost!"));
		exit(1);
	}

	status_icon = gtk_status_icon_new_from_pixbuf(pixbuf);
	inter.status_icon = status_icon;
	g_object_unref(pixbuf);
	screen = gdk_screen_get_default();
	gtk_status_icon_set_screen(status_icon, screen);
	gtk_status_icon_set_tooltip(status_icon, _("iptux"));

	g_signal_connect(status_icon, "activate",
			 G_CALLBACK(StatusIconActivate), NULL);
	g_signal_connect(status_icon, "popup-menu",
			 G_CALLBACK(PopupWorkMenu), this);
}

void StatusIcon::UpdateTips()
{
	extern UdpData udt;
	extern struct interactive inter;
	char *ipstr;
	guint len;

	pthread_mutex_lock(udt.MutexQuote());
	if (len = g_queue_get_length(udt.MsgqueueQuote())) {
		gtk_status_icon_set_blinking(inter.status_icon, TRUE);
		ipstr = g_strdup_printf(_("Undealt: %u messages"), len);
		gtk_status_icon_set_tooltip(inter.status_icon, ipstr);
	} else {
		gtk_status_icon_set_blinking(inter.status_icon, FALSE);
		ipstr = get_sys_host_addr_string(inter.udpsock);
		gtk_status_icon_set_tooltip(inter.status_icon,
					    ipstr ? ipstr : _("iptux"));
	}
	free(ipstr);
	pthread_mutex_unlock(udt.MutexQuote());
}

GtkWidget *StatusIcon::CreatePopupMenu()
{
	extern struct interactive inter;
	GtkWidget *menu, *menu_item;
	GtkWidget *image;

	menu = gtk_menu_new();
	gtk_widget_show(menu);

	if (GTK_WIDGET_VISIBLE(inter.window))
		menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Hide"));
	else
		menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Show"));
// 	image = gtk_image_new_from_file(__TIP_DIR "/desk.png");
// 	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(SwitchWindowMode), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	gtk_widget_show(menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Transport"));
	image = gtk_image_new_from_stock(GTK_STOCK_CONNECT, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(Transport::TransportEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Settings"));
	image = gtk_image_new_from_stock(GTK_STOCK_PREFERENCES,
						 GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(IptuxSetting::SettingEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Public"));
// 	image = gtk_image_new_from_file(__TIP_DIR "/share.png");
// 	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(ShareFile::ShareEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Group"));
// 	image = gtk_image_new_from_file(__TIP_DIR "/net.png");
// 	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(DialogGroup::DialogEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	gtk_widget_show(menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Detect"));
// 	image = gtk_image_new_from_file(__TIP_DIR "/detect.png");
// 	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(DetectPal::DetectEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Quit"));
	image = gtk_image_new_from_stock(GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(iptux_gui_quit), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	return menu;
}

void StatusIcon::SwitchWindowMode()
{
	extern struct interactive inter;
	GdkPixbuf *pixbuf;

	if (GTK_WIDGET_VISIBLE(inter.window)) {
		gtk_widget_hide(inter.window);
		pixbuf = gdk_pixbuf_new_from_file_at_size(
				__LOGO_DIR"/i-tux.png", 20, 20, NULL);
		if (pixbuf) {
			gtk_status_icon_set_from_pixbuf(inter.status_icon, pixbuf);
			g_object_unref(pixbuf);
		} else
			pwarning(Fail, "%s \"" __LOGO_DIR "/i-tux.png\" %s",
				 _("The notify icon"), _("is lost!"));
	} else {
		gtk_widget_show(inter.window);
		pixbuf = gdk_pixbuf_new_from_file_at_size(
				__LOGO_DIR "/ip-tux.png", 20, 20, NULL);
		if (pixbuf) {
			gtk_status_icon_set_from_pixbuf(inter.status_icon, pixbuf);
			g_object_unref(pixbuf);
		} else
			pwarning(Fail, "%s \"" __LOGO_DIR "/ip-tux.png\" %s",
				 _("The notify icon"), _("is lost!"));
	}
}

void StatusIcon::StatusIconActivate()
{
	extern UdpData udt;
	Pal *pal;

	pthread_mutex_lock(udt.MutexQuote());
	pal = (Pal *) g_queue_peek_head(udt.MsgqueueQuote());
	pthread_mutex_unlock(udt.MutexQuote());
	if (pal)
		DialogPeer::DialogEntry(pal);
	else
		SwitchWindowMode();
}

void StatusIcon::PopupWorkMenu(GtkStatusIcon * status_icon, guint button,
			       guint activate_time, gpointer data)
{
	StatusIcon *stic;

	stic = (StatusIcon *) data;
	gtk_menu_popup(GTK_MENU(stic->CreatePopupMenu()), NULL, NULL,
				       NULL, NULL, button, activate_time);
}
