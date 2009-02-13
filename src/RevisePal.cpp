//
// C++ Implementation: RevisePal
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "RevisePal.h"
#include "MainWindow.h"
#include "IptuxSetting.h"
#include "UdpData.h"
#include "my_entry.h"
#include "baling.h"
#include "utils.h"

 RevisePal::RevisePal(gpointer data):pal((Pal *) data),
revise(NULL), icon_model(NULL), name(NULL), group(NULL),
encode(NULL), icon(NULL), compatible(NULL)
{
}

RevisePal::~RevisePal()
{
	g_object_unref(icon_model);
}

void RevisePal::ReviseEntry(gpointer data)
{
	RevisePal rp(data);

	rp.InitRevise();
	rp.CreateRevise();
	rp.RunRevise();
}

void RevisePal::InitRevise()
{
	icon_model = IptuxSetting::CreateIconModel();
}

void RevisePal::CreateRevise()
{
	extern struct interactive inter;
	GtkWidget *box, *button;

	revise = gtk_dialog_new_with_buttons(_("Change pal's information"),
					     GTK_WINDOW(inter.window),
					     GTK_DIALOG_MODAL,
					     _("Cancel"), GTK_RESPONSE_CANCEL,
					     _("OK"), GTK_RESPONSE_OK, NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(revise), GTK_RESPONSE_OK);

	box = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(revise)->vbox),
					   box, FALSE, FALSE, 0);
	name = create_label(_("Pal's nickname:"));
	gtk_box_pack_start(GTK_BOX(box), name, FALSE, FALSE, 0);
	name = my_entry::create_entry(pal->NameQuote(),
				      _("Please input pal's new nickname!"),
				      FALSE);
	gtk_box_pack_start(GTK_BOX(box), name, TRUE, TRUE, 0);

	box = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(revise)->vbox),
			   box, FALSE, FALSE, 0);
	group = create_label(_("Pal's group name:"));
	gtk_box_pack_start(GTK_BOX(box), group, FALSE, FALSE, 0);
	group = my_entry::create_entry(pal->GroupQuote(),
				       _("Please input pal's new group name!"),
				       FALSE);
	gtk_box_pack_start(GTK_BOX(box), group, TRUE, TRUE, 0);

	box = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(revise)->vbox),
			   box, FALSE, FALSE, 0);
	encode = create_label(_("System encode:"));
	gtk_box_pack_start(GTK_BOX(box), encode, FALSE, FALSE, 0);
	encode = my_entry::create_entry(pal->EncodeQuote(),
				_("you must understand what you are doing!"),
				FALSE);
	gtk_box_pack_start(GTK_BOX(box), encode, TRUE, TRUE, 0);

	box = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(revise)->vbox),
					   box, FALSE, FALSE, 0);
	icon = create_label(_("Head portrait:"));
	gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 0);
	icon = IptuxSetting::CreateComboBoxWithModel(icon_model,
					 pal->IconfileQuote());
	gtk_box_pack_start(GTK_BOX(box), icon, TRUE, TRUE, 0);
	button = create_button("...");
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(IptuxSetting::AddPalIcon), icon);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);

	box = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(revise)->vbox),
			   box, FALSE, FALSE, 10);
	compatible = gtk_check_button_new_with_label(
				_("Be compatible with iptux protocol"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(compatible),
				     FLAG_ISSET(pal->FlagsQuote(), 0));
	gtk_widget_show(compatible);
	gtk_box_pack_start(GTK_BOX(box), compatible, FALSE, FALSE, 0);
}

void RevisePal::RunRevise()
{
	int result;

	result = gtk_dialog_run(GTK_DIALOG(revise));
	if (result == GTK_RESPONSE_OK)
		ApplyRevise();
	gtk_widget_destroy(revise);
}

void RevisePal::ApplyRevise()
{
	extern MainWindow *mwp;
	char buf[MAX_BUF];
	GtkTreeIter iter;
	gint active;

	free(pal->NameQuote());
	pal->NameQuote() = gtk_editable_get_chars(GTK_EDITABLE(name), 0, -1);
	free(pal->GroupQuote());
	pal->GroupQuote() = gtk_editable_get_chars(GTK_EDITABLE(group), 0, -1);
	free(pal->EncodeQuote());
	pal->EncodeQuote() = gtk_editable_get_chars(GTK_EDITABLE(encode), 0, -1);

	active = gtk_combo_box_get_active(GTK_COMBO_BOX(icon));
	snprintf(buf, MAX_BUF, "%d", active);
	gtk_tree_model_get_iter_from_string(icon_model, &iter, buf);
	free(pal->IconfileQuote());
	gtk_tree_model_get(icon_model, &iter, 1, &pal->IconfileQuote(), -1);
	if (mwp->PalGetModelIter(pal, &iter))
		mwp->SetValueToModel(pal, &iter);

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(compatible)))
		FLAG_SET(pal->FlagsQuote(), 0);
	else
		FLAG_CLR(pal->FlagsQuote(), 0);

	FLAG_SET(pal->FlagsQuote(), 2);
}
