//
// C++ Implementation: DetectPal
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "DetectPal.h"
#include "Command.h"
#include "my_entry.h"
#include "baling.h"
#include "output.h"

GtkWidget *DetectPal::detect = NULL;
 DetectPal::DetectPal():ipstr(NULL)
{
}

DetectPal::~DetectPal()
{
}

void DetectPal::DetectEntry()
{
	DetectPal dp;

	if (DetectPal::CheckExsit())
		return;
	dp.CreateDetect();
	dp.RunDetect();
}

void DetectPal::CreateDetect()
{
	extern struct interactive inter;
	GtkWidget *frame;

	detect = gtk_dialog_new_with_buttons(_("Detect the pals"),
				    GTK_WINDOW(inter.window), GTK_DIALOG_MODAL,
				    _("Cancel"), GTK_RESPONSE_CANCEL,
				    _("Detect"), GTK_RESPONSE_ACCEPT, NULL);
	g_signal_connect(detect, "destroy", G_CALLBACK(DetectDestroy), NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(detect), GTK_RESPONSE_ACCEPT);

	frame = create_frame(_("Please input a legal address of IPv4:"));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(detect)->vbox), frame,
						   TRUE, TRUE, 0);
	ipstr = my_entry::create_entry(NULL,
			     _("Please input a legal address of IPv4!"), true);
	gtk_entry_set_max_length(GTK_ENTRY(ipstr), INET_ADDRSTRLEN);
	gtk_container_add(GTK_CONTAINER(frame), ipstr);
	gtk_widget_grab_focus(ipstr);
}

void DetectPal::RunDetect()
{
	gint result;

	do {
		result = gtk_dialog_run(GTK_DIALOG(detect));
		if (result == GTK_RESPONSE_ACCEPT)
			SendDetectPacket();
	} while (result == GTK_RESPONSE_ACCEPT);
	gtk_widget_destroy(detect);
}

void DetectPal::SendDetectPacket()
{
	extern struct interactive inter;
	const char *text;
	Command cmd;
	in_addr_t ipv4;

	text = gtk_entry_get_text(GTK_ENTRY(ipstr));
	if (inet_pton(AF_INET, text, &ipv4) <= 0) {
		pop_warning(detect, ipstr, _("\nThe address %s is illegal!"),
								    text);
		gtk_editable_select_region(GTK_EDITABLE(ipstr), 0, -1);
		return;
	}

	cmd.SendDetectPacket(inter.udpsock, ipv4);
	pop_info(detect, ipstr, _("\nSending a notice to %s is done!"), text);
	gtk_entry_set_text(GTK_ENTRY(ipstr), "");
}

bool DetectPal::CheckExsit()
{
	if (!detect)
		return false;
	gtk_window_present(GTK_WINDOW(detect));
	return true;
}

void DetectPal::DetectDestroy()
{
	detect = NULL;
}
