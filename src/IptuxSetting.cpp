//
// C++ Implementation: IptuxSetting
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "IptuxSetting.h"
#include "Control.h"
#include "my_entry.h"
#include "my_chooser.h"
#include "UdpData.h"
#include "DialogGroup.h"
#include "Command.h"
#include "Sound.h"
#include "Pal.h"
#include "support.h"
#include "output.h"
#include "baling.h"
#include "utils.h"

GtkWidget *IptuxSetting::setting = NULL;
 IptuxSetting::IptuxSetting():icon_model(NULL), snd_model(NULL),
ip_model(NULL), myname(NULL), mygroup(NULL), myicon(NULL),
save_path(NULL), ad(NULL), sign(NULL), encode(NULL),
palicon(NULL), font(NULL), memory(NULL), etrkey(NULL),
tidy(NULL), log(NULL), black(NULL), proof(NULL), sound(NULL),
volume(NULL), entry1(NULL), entry2(NULL), ipseg_view(NULL)
{
}

IptuxSetting::~IptuxSetting()
{
	g_object_unref(icon_model);
#ifdef HAVE_GST
	g_object_unref(snd_model);
#endif
	g_object_unref(ip_model);
}

void IptuxSetting::SettingEntry()
{
	IptuxSetting *ipst;

	if (IptuxSetting::CheckExist())
		return;
	ipst = new IptuxSetting;
	ipst->InitSetting();
	ipst->CreateSetting();
}

void IptuxSetting::InitSetting()
{
	icon_model = CreateIconModel();
#ifdef HAVE_GST
	snd_model = CreateSndModel();
#endif
	ip_model = CreateIpModel();
}

void IptuxSetting::CreateSetting()
{
	GtkWidget *box;
	GtkWidget *notebook, *hbb;

	setting = create_window(_("Iptux Settings"), 132, 100);
	gtk_container_set_border_width(GTK_CONTAINER(setting), 5);
	g_signal_connect_swapped(setting, "destroy",
				 G_CALLBACK(SettingDestroy), this);
	box = create_box();
	gtk_container_add(GTK_CONTAINER(setting), box);

	notebook = gtk_notebook_new();
	gtk_widget_show(notebook);
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_LEFT);
	gtk_box_pack_start(GTK_BOX(box), notebook, TRUE, TRUE, 0);
	CreatePerson(notebook);
	CreateSystem(notebook);
#ifdef HAVE_GST
	CreateSound(notebook);
#endif
	CreateIpseg(notebook);

	hbb = create_button_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbb, FALSE, FALSE, 0);
	CreateFuncButton(hbb);
}

void IptuxSetting::CreatePerson(GtkWidget * note)
{
	extern Control ctr;
	char path[MAX_PATHBUF];
	GtkWidget *label, *frame, *sw, *button;
	GtkWidget *box, *hbox;
	GdkPixbuf *pixbuf;

	box = create_box();
	label = create_label(_("User"));
	gtk_notebook_append_page(GTK_NOTEBOOK(note), box, label);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
	myname = create_label(_("Nick:"));
	gtk_box_pack_start(GTK_BOX(hbox), myname, FALSE, FALSE, 0);
	myname = my_entry::create_entry(ctr.myname,
				      _("Please Input Your Nickname!"), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), myname, TRUE, TRUE, 0);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
	mygroup = create_label(_("Group:"));
	gtk_box_pack_start(GTK_BOX(hbox), mygroup, FALSE, FALSE, 0);
	mygroup = my_entry::create_entry(ctr.mygroup,
				    _("Please Input your Group name!"), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), mygroup, TRUE, TRUE, 0);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
	myicon = create_label(_("Buddy Icon:"));
	gtk_box_pack_start(GTK_BOX(hbox), myicon, FALSE, FALSE, 0);
	myicon = CreateComboBoxWithModel(icon_model, ctr.myicon);
	gtk_box_pack_start(GTK_BOX(hbox), myicon, TRUE, TRUE, 0);
	button = create_button("...");
	g_signal_connect_swapped(button, "clicked",
				    G_CALLBACK(AddPalIcon), myicon);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
	save_path = create_label(_("Received Files Location:"));
	gtk_box_pack_start(GTK_BOX(hbox), save_path, FALSE, FALSE, 0);
	save_path = CreateArchiveChooser();
	gtk_box_pack_start(GTK_BOX(hbox), save_path, TRUE, TRUE, 0);

	/***************************/
	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 5);
	/***************************/
	frame = create_frame(_("Photo"));
	gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 3);
	button = gtk_button_new();
	gtk_widget_show(button);
	gtk_widget_set_size_request(button, MAX_PREVIEWSIZE, MAX_PREVIEWSIZE);
	gtk_container_add(GTK_CONTAINER(frame), button);
	ad = gtk_image_new();
	gtk_widget_show(ad);
	gtk_container_add(GTK_CONTAINER(button), ad);
	snprintf(path, MAX_PATHBUF, "%s" COMPLEX_PATH "/ad",
					    g_get_user_config_dir());
	if ( (pixbuf = gdk_pixbuf_new_from_file(path, NULL))) {
		pixbuf_shrink_scale_1(&pixbuf, MAX_PREVIEWSIZE, MAX_PREVIEWSIZE);
		gtk_image_set_from_pixbuf(GTK_IMAGE(ad), pixbuf);
		g_object_unref(pixbuf);
	}
	g_signal_connect_swapped(button, "clicked",
			    G_CALLBACK(ChoosePortrait), ad);

	frame = create_frame(_("Sign Words:"));
	gtk_box_pack_end(GTK_BOX(hbox), frame, TRUE, TRUE, 3);
	sw = create_scrolled_window();
	gtk_container_add(GTK_CONTAINER(frame), sw);
	sign = create_text_view();
	gtk_container_add(GTK_CONTAINER(sw), sign);
	gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(sign)),
						 ctr.sign ? ctr.sign : "", -1);
}

void IptuxSetting::CreateSystem(GtkWidget * note)
{
	extern Control ctr;
	GtkWidget *label, *button;
	GtkWidget *box, *hbox;

	box = create_box();
	label = create_label(_("System"));
	gtk_notebook_append_page(GTK_NOTEBOOK(note), box, label);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
	encode = create_label(_("System Default Encode:"));
	gtk_box_pack_start(GTK_BOX(hbox), encode, FALSE, FALSE, 0);
	encode = my_entry::create_entry(ctr.encode,
			    _("(Before modified,"
			    "be SURE to know what you are doing)"),
			    FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), encode, TRUE, TRUE, 0);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
	palicon = create_label(_("Default Buddy Icon"));
	gtk_box_pack_start(GTK_BOX(hbox), palicon, FALSE, FALSE, 0);
	palicon = CreateComboBoxWithModel(icon_model, ctr.palicon);
	gtk_box_pack_start(GTK_BOX(hbox), palicon, TRUE, TRUE, 0);
	button = create_button("...");
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(AddPalIcon), palicon);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
	font = create_label(_("Panel Font:"));
	gtk_box_pack_start(GTK_BOX(hbox), font, FALSE, FALSE, 0);
	font = CreateFontChooser();
	gtk_box_pack_start(GTK_BOX(hbox), font, TRUE, TRUE, 0);

	memory = gtk_check_button_new_with_label(
			_("Minimise Memory Usage(NOT Recommended)"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(memory),
				     FLAG_ISSET(ctr.flags, 5));
	gtk_widget_show(memory);
	gtk_box_pack_start(GTK_BOX(box), memory, FALSE, FALSE, 0);

	etrkey = gtk_check_button_new_with_label(
			_("Use 'Enter' to Send Message"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(etrkey),
				     FLAG_ISSET(ctr.flags, 4));
	gtk_widget_show(etrkey);
	gtk_box_pack_start(GTK_BOX(box), etrkey, FALSE, FALSE, 0);

	tidy = gtk_check_button_new_with_label(
		       _("Automaticlly Clean up Chatting History"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tidy),
					     FLAG_ISSET(ctr.flags, 3));
	gtk_widget_show(tidy);
	gtk_box_pack_start(GTK_BOX(box), tidy, FALSE, FALSE, 0);

	log = gtk_check_button_new_with_label(
			      _("Log for the Messages."));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(log),
				     FLAG_ISSET(ctr.flags, 2));
	gtk_widget_show(log);
	gtk_box_pack_start(GTK_BOX(box), log, FALSE, FALSE, 0);

	black = gtk_check_button_new_with_label(
			     _("Use Blacklist (NOT recommended)"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(black),
				     FLAG_ISSET(ctr.flags, 1));
	gtk_widget_show(black);
	gtk_box_pack_start(GTK_BOX(box), black, FALSE, FALSE, 0);

	proof = gtk_check_button_new_with_label(
			     _("Ignore Requests for Shared Files"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(proof),
				     FLAG_ISSET(ctr.flags, 0));
	gtk_widget_show(proof);
	gtk_box_pack_start(GTK_BOX(box), proof, FALSE, FALSE, 0);
}

void IptuxSetting::CreateSound(GtkWidget * note)
{
	extern Control ctr;
	GtkWidget *box, *label;
	GtkWidget *frame, *vbox, *sw, *view;
	GtkWidget *hbox, *chooser, *button;

	box = create_box();
	label = create_label(_("Sound"));
	gtk_notebook_append_page(GTK_NOTEBOOK(note), box, label);

	sound = gtk_check_button_new_with_label(
			_("Sound Supported"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sound),
				     FLAG_ISSET(ctr.sndfgs, 0));
	gtk_widget_show(sound);
	gtk_box_pack_start(GTK_BOX(box), sound, FALSE, FALSE, 3);

	hbox = create_box(FALSE);
	AdjustSensitive(sound, hbox);
	g_signal_connect(sound, "toggled", G_CALLBACK(AdjustSensitive), hbox);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
	label = create_label(_("Volume Control: "));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	volume = gtk_hscale_new_with_range(0.0, 1.0, 0.01);
	g_signal_connect(volume, "value-changed", G_CALLBACK(AdjustVolume), NULL);
	gtk_range_set_value(GTK_RANGE(volume), ctr.volume);
	gtk_scale_set_draw_value(GTK_SCALE(volume), FALSE);
	gtk_widget_show(volume);
	gtk_box_pack_start(GTK_BOX(hbox), volume, TRUE, TRUE, 0);

	frame = create_frame(_("Sound Event"));
	AdjustSensitive(sound, frame);
	g_signal_connect(sound, "toggled", G_CALLBACK(AdjustSensitive), frame);
	gtk_box_pack_start(GTK_BOX(box), frame, TRUE, TRUE, 3);
	vbox = create_box();
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	sw = create_scrolled_window();
	gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 0);
	view = CreateSndView();
	gtk_container_add(GTK_CONTAINER(sw), view);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	chooser = CreateSndChooser();
	gtk_box_pack_start(GTK_BOX(hbox), chooser, TRUE, TRUE, 5);
	button = create_button(_("Play"));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(PlayTesting), chooser);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	button = create_button(_("Stop"));
	g_signal_connect(button, "clicked", G_CALLBACK(StopTesting), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	g_signal_connect(view, "cursor-changed",
			 G_CALLBACK(CursorItemChanged), chooser);
	g_signal_connect(chooser, "file-set",
			 G_CALLBACK(ChooserResetView), view);
}

void IptuxSetting::CreateIpseg(GtkWidget * note)
{
	char buf[MAX_BUF];
	GtkWidget *box, *label;
	GtkWidget *hbox, *vbox, *button;
	GtkWidget *frame, *sw;

	box = create_box();
	label = create_label(_("Network"));
	gtk_notebook_append_page(GTK_NOTEBOOK(note), box, label);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
	entry1 = create_label(_("Begin:"));
	gtk_box_pack_start(GTK_BOX(hbox), entry1, FALSE, FALSE, 0);
	entry1 = my_entry::create_entry(NULL, _("begin IPv4"), TRUE);
	gtk_box_pack_start(GTK_BOX(hbox), entry1, TRUE, TRUE, 0);
	entry2 = create_label(_("End:"));
	gtk_box_pack_start(GTK_BOX(hbox), entry2, FALSE, FALSE, 0);
	entry2 = my_entry::create_entry(NULL, _("end IPv4"), TRUE);
	gtk_box_pack_start(GTK_BOX(hbox), entry2, TRUE, TRUE, 0);

	hbox = create_button_box(FALSE);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox), GTK_BUTTONBOX_SPREAD);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
	snprintf(buf, MAX_BUF, "%s↓↓", _("Add"));
	button = create_button(buf);
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(ClickAddIpseg), this);
	gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	snprintf(buf, MAX_BUF, "%s↑↑", _("Delete"));
	button = create_button(buf);
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(ClickDelIpseg), this);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	frame = create_frame(_("Prefered IP section:"));
	gtk_box_pack_start(GTK_BOX(box), frame, TRUE, TRUE, 5);
	hbox = create_box(FALSE);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	sw = create_scrolled_window();
	gtk_box_pack_start(GTK_BOX(hbox), sw, TRUE, TRUE, 0);
	ipseg_view = CreateIpsegView();
	gtk_container_add(GTK_CONTAINER(sw), ipseg_view);
	vbox = create_box();
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
	button = create_button(_("Import"));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(ImportNetSegment), this);
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
	button = create_button(_("Export"));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(ExportNetSegment), this);
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
	button = create_button(_("Clear"));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(gtk_list_store_clear), ip_model);
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
}

void IptuxSetting::CreateFuncButton(GtkWidget * hbb)
{
	GtkWidget *button;

	button = create_button(_("OK"));
	g_signal_connect_swapped(button, "clicked", G_CALLBACK(ClickOk), this);
	gtk_box_pack_end(GTK_BOX(hbb), button, FALSE, FALSE, 1);
	button = create_button(_("Apply"));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(ClickApply), this);
	gtk_box_pack_end(GTK_BOX(hbb), button, FALSE, FALSE, 1);
	button = create_button(_("Cancel"));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(gtk_widget_destroy), setting);
	gtk_box_pack_end(GTK_BOX(hbb), button, FALSE, FALSE, 1);
}

//sound,0 play,1 note,2 path
GtkTreeModel *IptuxSetting::CreateSndModel()
{
	extern Control ctr;
	GtkListStore *model;
	GtkTreeIter iter;

	model = gtk_list_store_new(3, G_TYPE_BOOLEAN,
				   G_TYPE_STRING, G_TYPE_STRING);
	gtk_list_store_append(model, &iter);
	gtk_list_store_set(model, &iter, 0, FLAG_ISSET(ctr.sndfgs, 1),
			   1, _("Message Received"), 2, ctr.msgtip, -1);
	gtk_list_store_append(model, &iter);
	gtk_list_store_set(model, &iter, 0, FLAG_ISSET(ctr.sndfgs, 2),
			   1, _("Transport Finished"), 2, ctr.transtip, -1);

	return GTK_TREE_MODEL(model);
}

//IP 3,0 ip,1 ip,2 describe
GtkTreeModel *IptuxSetting::CreateIpModel()
{
	extern Control ctr;
	GtkListStore *model;
	GtkTreeIter iter;
	NetSegment *ns;
	GSList *tmp;

	model = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING,
						   G_TYPE_STRING);
	pthread_mutex_lock(&ctr.mutex);
	tmp = ctr.netseg;
	while (tmp) {
		ns = (NetSegment *) tmp->data;
		gtk_list_store_append(model, &iter);
		gtk_list_store_set(model, &iter, 0, ns->startip, 1, ns->endip,
						   2, ns->describe, -1);
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&ctr.mutex);

	return GTK_TREE_MODEL(model);
}

GtkWidget *IptuxSetting::CreateFontChooser()
{
	extern Control ctr;
	GtkWidget *chooser;

	chooser = gtk_font_button_new_with_font(ctr.font);
	gtk_font_button_set_show_style(GTK_FONT_BUTTON(chooser), TRUE);
	gtk_font_button_set_show_size(GTK_FONT_BUTTON(chooser), TRUE);
	gtk_font_button_set_use_font(GTK_FONT_BUTTON(chooser), TRUE);
	gtk_font_button_set_use_size(GTK_FONT_BUTTON(chooser), TRUE);
	gtk_font_button_set_title(GTK_FONT_BUTTON(chooser),
						  _("Please choose a font"));
	gtk_widget_show(chooser);

	return chooser;
}

GtkWidget *IptuxSetting::CreateSndChooser()
{
	extern Control ctr;
	GtkWidget *chooser;

	chooser = gtk_file_chooser_button_new(_("Please choose a sound file"),
					      GTK_FILE_CHOOSER_ACTION_OPEN);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(chooser), TRUE);
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(chooser), ctr.msgtip);
	gtk_widget_show(chooser);

	return chooser;
}

GtkWidget *IptuxSetting::CreateSndView()
{
	GtkWidget *view;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;
	GtkTreePath *path;

	view = gtk_tree_view_new_with_model(snd_model);
	g_signal_connect_swapped(view, "button-press-event",
			 G_CALLBACK(DialogGroup::PopupPickMenu), snd_model);
	gtk_widget_show(view);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("Play"));
	renderer = gtk_cell_renderer_toggle_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "active", 0, NULL);
	g_signal_connect_swapped(renderer, "toggled",
			 G_CALLBACK(DialogGroup::ViewToggleChange), snd_model);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("Event"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	/* 函数ChooserResetView()要求必须选择一项 */
	path = gtk_tree_path_new_from_string("0");
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	gtk_tree_selection_select_path(selection, path);
	gtk_tree_path_free(path);

	return view;
}

GtkWidget *IptuxSetting::CreateIpsegView()
{
	GtkWidget *view;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;

	view = gtk_tree_view_new_with_model(ip_model);
	gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(view), TRUE);
	gtk_widget_show(view);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("Begin IP"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("End IP"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_column_set_title(column, _("Describe"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 2, NULL);
	g_object_set(renderer, "editable", TRUE, NULL);
	g_signal_connect(renderer, "edited", G_CALLBACK(CellEditText), ip_model);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	return view;
}

bool IptuxSetting::CheckExist()
{
	if (!setting)
		return false;
	gtk_window_present(GTK_WINDOW(setting));
	return true;
}

GtkWidget *IptuxSetting::CreateArchiveChooser()
{
	extern Control ctr;
	GtkWidget *chooser;

	chooser = gtk_file_chooser_button_new(_("Save file to"),
				      GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_do_overwrite_confirmation(
				      GTK_FILE_CHOOSER(chooser), TRUE);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(chooser), TRUE);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), ctr.path);
	gtk_widget_show(chooser);

	return chooser;
}

//头像 2,0 pixbuf,1 iconfile
GtkTreeModel *IptuxSetting::CreateIconModel()
{
	extern Control ctr;
	GtkListStore *model;
	GtkTreeIter iter;
	GSList *tmp;
	SysIcon *si;

	model = gtk_list_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_STRING);
	pthread_mutex_lock(&ctr.mutex);
	tmp = ctr.iconlist;
	while (tmp) {
		si = (SysIcon *) tmp->data;
		if (!si->pixbuf)
			si->pixbuf = gdk_pixbuf_new_from_file_at_size(
					   si->pathname, MAX_ICONSIZE,
					   MAX_ICONSIZE, NULL);
		if (si->pixbuf) {
			gtk_list_store_append(model, &iter);
			gtk_list_store_set(model, &iter, 0, si->pixbuf,
						   1, si->pathname, -1);
		}
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&ctr.mutex);

	return GTK_TREE_MODEL(model);
}

GtkWidget *IptuxSetting::CreateComboBoxWithModel(GtkTreeModel * model,
						 gchar * iconfile)
{
	GtkWidget *combo;
	GtkCellRenderer *renderer;
	gint active;

	combo = gtk_combo_box_new_with_model(model);
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer,
						       "pixbuf", 0, NULL);
	gtk_combo_box_set_wrap_width(GTK_COMBO_BOX(combo), 6);
	active = FileGetItemPos(iconfile, model);
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo), active);
	gtk_widget_show(combo);

	return combo;
}

gint IptuxSetting::FileGetItemPos(const char *filename, GtkTreeModel * model)
{
	GdkPixbuf *pixbuf;
	GtkTreeIter iter;
	gchar *tmp;
	gint pos;

	pos = 0;
	if (gtk_tree_model_get_iter_first(model, &iter)) {
		do {
			gtk_tree_model_get(model, &iter, 1, &tmp, -1);
			if (strcmp(filename, tmp) == 0) {
				g_free(tmp);
				return pos;
			}
			g_free(tmp), pos++;
		} while (gtk_tree_model_iter_next(model, &iter));
	}
	if (access(filename, F_OK) != 0
		   || !(pixbuf = gdk_pixbuf_new_from_file_at_size(filename,
				MAX_ICONSIZE, MAX_ICONSIZE, NULL)))
		return -1;
	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, pixbuf,
						   1, filename, -1);
	g_object_unref(pixbuf);

	return pos;
}

void IptuxSetting::ObtainPerson()
{
	extern Control ctr;
	GtkTextIter start, end;
	GtkTextBuffer *buffer;
	GdkPixbuf *pixbuf;
	GtkTreeIter iter;
	char buf[MAX_PATHBUF];
	const char *text;
	gint active;

	text = gtk_entry_get_text(GTK_ENTRY(myname));
	free(ctr.myname);
	ctr.myname = Strdup(text);

	text = gtk_entry_get_text(GTK_ENTRY(mygroup));
	free(ctr.mygroup);
	ctr.mygroup = Strdup(text);

	active = gtk_combo_box_get_active(GTK_COMBO_BOX(myicon));
	snprintf(buf, MAX_PATHBUF, "%d", active);
	gtk_tree_model_get_iter_from_string(icon_model, &iter, buf);
	free(ctr.myicon);
	gtk_tree_model_get(icon_model, &iter, 1, &ctr.myicon, -1);
	if (strncmp(ctr.myicon, __ICON_PATH, strlen(__ICON_PATH))) {
		snprintf(buf, MAX_PATHBUF, "%s" COMPLEX_PATH "/icon",
						 g_get_user_config_dir());
		pixbuf = gdk_pixbuf_new_from_file_at_size(ctr.myicon,
					  MAX_ICONSIZE, MAX_ICONSIZE, NULL);
		gdk_pixbuf_save(pixbuf, buf, "png", NULL, NULL);
		g_object_unref(pixbuf), free(ctr.myicon);
		ctr.myicon = Strdup(buf);
	}

	free(ctr.path);
	ctr.path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(save_path));

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(sign));
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	free(ctr.sign);
	ctr.sign = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
}

void IptuxSetting::ObtainSystem()
{
	extern Control ctr;
	GtkTreeIter iter;
	char buf[MAX_BUF];
	const char *text;
	gint active;

	text = gtk_entry_get_text(GTK_ENTRY(encode));
	free(ctr.encode);
	ctr.encode = Strdup(text);

	active = gtk_combo_box_get_active(GTK_COMBO_BOX(palicon));
	snprintf(buf, MAX_BUF, "%d", active);
	gtk_tree_model_get_iter_from_string(icon_model, &iter, buf);
	free(ctr.palicon);
	gtk_tree_model_get(icon_model, &iter, 1, &ctr.palicon, -1);

	text = gtk_font_button_get_font_name(GTK_FONT_BUTTON(font));
	free(ctr.font);
	ctr.font = Strdup(text);

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(memory)))
		FLAG_SET(ctr.flags, 5);
	else
		FLAG_CLR(ctr.flags, 5);

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(etrkey)))
		FLAG_SET(ctr.flags, 4);
	else
		FLAG_CLR(ctr.flags, 4);

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tidy)))
		FLAG_SET(ctr.flags, 3);
	else
		FLAG_CLR(ctr.flags, 3);

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(log)))
		FLAG_SET(ctr.flags, 2);
	else
		FLAG_CLR(ctr.flags, 2);

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(black)))
		FLAG_SET(ctr.flags, 1);
	else
		FLAG_CLR(ctr.flags, 1);

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(proof)))
		FLAG_SET(ctr.flags, 0);
	else
		FLAG_CLR(ctr.flags, 0);
}

void IptuxSetting::ObtainSound()
{
	extern Control ctr;
	GtkTreeIter iter;
	gboolean active;
	gchar *path;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sound)))
		FLAG_SET(ctr.sndfgs, 0);
	else
		FLAG_CLR(ctr.sndfgs, 0);

	ctr.volume = gtk_range_get_value(GTK_RANGE(volume));

	/* 注意与 Control 中成员变量联合检查 */
	gtk_tree_model_get_iter_from_string(snd_model, &iter, "0");
	gtk_tree_model_get(snd_model, &iter, 0, &active, 2, &path, -1);
	if (active)
		FLAG_SET(ctr.sndfgs, 1);
	else
		FLAG_CLR(ctr.sndfgs, 1);
	free(ctr.msgtip);
	ctr.msgtip = path;

	gtk_tree_model_get_iter_from_string(snd_model, &iter, "1");
	gtk_tree_model_get(snd_model, &iter, 0, &active, 2, &path, -1);
	if (active)
		FLAG_SET(ctr.sndfgs, 2);
	else
		FLAG_CLR(ctr.sndfgs, 2);
	free(ctr.transtip);
	ctr.transtip = path;
}

void IptuxSetting::ObtainIpseg()
{
	extern Control ctr;
	GtkTreeIter iter;
	NetSegment *ns;

	pthread_mutex_lock(&ctr.mutex);
	g_slist_foreach(ctr.netseg, GFunc(remove_foreach),
			GINT_TO_POINTER(NETSEGMENT));
	g_slist_free(ctr.netseg), ctr.netseg = NULL;
	if (gtk_tree_model_get_iter_first(ip_model, &iter)) {
		do {
			ns = new NetSegment(NULL, NULL, NULL);
			gtk_tree_model_get(ip_model, &iter, 0, &ns->startip,
					   1, &ns->endip, 2, &ns->describe, -1);
			ns->describe = ns->describe ? ns->describe : Strdup("");
			ctr.netseg = g_slist_append(ctr.netseg, ns);
		} while (gtk_tree_model_iter_next(ip_model, &iter));
	}
	pthread_mutex_unlock(&ctr.mutex);
}

void IptuxSetting::UpdateMyInfo()
{
	extern struct interactive inter;
	extern UdpData udt;
	Command cmd;
	GSList *tmp;

	pthread_mutex_lock(udt.MutexQuote());
	tmp = udt.PallistQuote();
	while (tmp) {
		cmd.SendAbsence(inter.udpsock, tmp->data);
		if (FLAG_ISSET(((Pal *) tmp->data)->FlagsQuote(), 0))
			thread_create(ThreadFunc(Pal::SendFeature),
					     tmp->data, false);
		tmp = tmp->next;
	}
	pthread_mutex_unlock(udt.MutexQuote());
}

//写出 TRUE,读入 FALSE
void IptuxSetting::UpdateNetSegment(const char *filename, GSList ** list,
				    bool dirc)
{
	char buf[3][MAX_BUF], *ptr, *lineptr;
	in_addr_t ipv4;
	NetSegment *ns;
	FILE *stream;
	GSList *tmp;
	size_t n;

	if (!(stream = Fopen(filename, dirc ? "w" : "r")))
		return;
	if (dirc) {
		fprintf(stream, "#This is a description table for net segment!");
		tmp = *list;
		while (tmp) {
			ns = (NetSegment *) tmp->data;
			fprintf(stream, "\n%s - %s\t//%s", ns->startip,
						  ns->endip, ns->describe);
			tmp = tmp->next;
		}
	} else {
		lineptr = NULL, n = 0;
		while (getline(&lineptr, &n, stream) != -1) {
			if ( (ptr = strchr(lineptr, '#')))
				*ptr = '\0';
			buf[2][0] = '\0';
			if (sscanf(lineptr, "%s - %s //%s", buf[0], buf[1], buf[2]) < 2
				    || inet_pton(AF_INET, buf[0], &ipv4) <= 0
				    || inet_pton(AF_INET, buf[1], &ipv4) <= 0)
				continue;
			ns = new NetSegment(Strdup(buf[0]), Strdup(buf[1]),
							    Strdup(buf[2]));
			*list = g_slist_append(*list, ns);
		}
		free(lineptr);
	}
	fclose(stream);
}

void IptuxSetting::AddPalIcon(GtkWidget * combo)
{
	GtkTreeModel *model;
	gchar *filename;
	gint active;

	if (!(filename = my_chooser::choose_file(
			      _("Choose a Buddy Icon"), setting)))
		return;
	model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo));
	active = FileGetItemPos(filename, model);
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo), active);
	g_free(filename);
}

void IptuxSetting::ChoosePortrait(GtkWidget * image)
{
	gchar path[MAX_PATHBUF], *filename;
	GdkPixbuf *pixbuf;

	if (!(filename = my_chooser::choose_file(
		      _("Choose Your Photo"), setting)))
		return;
	pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
	g_free(filename);
	if (pixbuf) {
		snprintf(path, MAX_PATHBUF, "%s" COMPLEX_PATH "/ad",
						 g_get_user_config_dir());
		pixbuf_shrink_scale_1(&pixbuf, MAX_ADSIZE, MAX_ADSIZE);
		gdk_pixbuf_save(pixbuf, path, "bmp", NULL, NULL);	//命中率极高，不妨直接保存
		pixbuf_shrink_scale_1(&pixbuf, MAX_PREVIEWSIZE,
						      MAX_PREVIEWSIZE);
		gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
		g_object_unref(pixbuf);
	}
}

void IptuxSetting::AdjustSensitive(GtkWidget *sound, GtkWidget *widget)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sound)))
		gtk_widget_set_sensitive(widget, TRUE);
	else
		gtk_widget_set_sensitive(widget, FALSE);
}

void IptuxSetting::AdjustVolume(GtkWidget *volume)
{
	extern Sound sound;
	gdouble value;

	value = gtk_range_get_value(GTK_RANGE(volume));
	sound.AdjustVolume(value);
}

void IptuxSetting::CursorItemChanged(GtkWidget *view, GtkWidget *chooser)
{
	GtkTreeModel *model;
	GtkTreePath *treepath;
	GtkTreeIter iter;
	gchar *path;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(view), &treepath, NULL);
	gtk_tree_model_get_iter(model, &iter, treepath);
	gtk_tree_path_free(treepath);
	gtk_tree_model_get(model, &iter, 2, &path, -1);
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(chooser), path);
	g_free(path);
}

void IptuxSetting::ChooserResetView(GtkWidget *chooser, GtkWidget *view)
{
	GtkTreeModel *model;
	GtkTreePath *treepath;
	GtkTreeIter iter;
	gchar *path;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(view), &treepath, NULL);
	if (!treepath)
		return;

	gtk_tree_model_get_iter(model, &iter, treepath);
	gtk_tree_path_free(treepath);
	path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 2, path, -1);
	g_free(path);
}

void IptuxSetting::PlayTesting(GtkWidget *chooser)
{
	extern Sound sound;
	gchar *path;

	path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
	sound.Playing(path);
	g_free(path);
}

void IptuxSetting::StopTesting()
{
	extern Sound sound;

	sound.Stop();
}

void IptuxSetting::ClickAddIpseg(gpointer data)
{
	const gchar *text1, *text2, *text;
	IptuxSetting *ipst;
	in_addr_t ip1, ip2;
	GtkTreeIter iter;
	int status;

	ipst = (IptuxSetting *) data;
	text1 = gtk_entry_get_text(GTK_ENTRY(ipst->entry1));
	status = inet_pton(AF_INET, text1, &ip1);
	if (status <= 0) {
		pop_warning(setting, NULL,
			    _("\nAddress %s Unacceptable!"), text1);
		return;
	}
	text2 = gtk_entry_get_text(GTK_ENTRY(ipst->entry2));
	status = inet_pton(AF_INET, text2, &ip2);
	if (status <= 0) {
		pop_warning(setting, NULL,
			    _("\nAddress %s Unacceptable!"), text2);
		return;
	}

	ip1 = ntohl(ip1), ip2 = ntohl(ip2);
	if (ip1 > ip2) {
		text = text1;
		text1 = text2;
		text2 = text;
	}

	gtk_list_store_append(GTK_LIST_STORE(ipst->ip_model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(ipst->ip_model), &iter,
						   0, text1, 1, text2, -1);
	gtk_entry_set_text(GTK_ENTRY(ipst->entry1), "\0");
	gtk_entry_set_text(GTK_ENTRY(ipst->entry2), "\0");
}

void IptuxSetting::ClickDelIpseg(gpointer data)
{
	gchar *ipstr1, *ipstr2;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	IptuxSetting *ipst;
	bool flag;

	ipst = (IptuxSetting *) data, flag = false;
	if (!gtk_tree_model_get_iter_first(ipst->ip_model, &iter))
		return;
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ipst->ipseg_view));
	do {
 mark:		if (gtk_tree_selection_iter_is_selected(selection, &iter))
		{
			if (!flag) {
				gtk_tree_model_get(ipst->ip_model, &iter, 0,
						   &ipstr1, 1, &ipstr2, -1);
				flag = true;
			}
			if (gtk_list_store_remove(
			    GTK_LIST_STORE(ipst->ip_model), &iter))
				goto mark;
			break;
		}
	} while (gtk_tree_model_iter_next(ipst->ip_model, &iter));

	gtk_entry_set_text(GTK_ENTRY(ipst->entry1), ipstr1);
	gtk_entry_set_text(GTK_ENTRY(ipst->entry2), ipstr2);
	g_free(ipstr1), g_free(ipstr2);
}

void IptuxSetting::CellEditText(GtkCellRendererText * renderer, gchar * path,
				gchar * new_text, GtkTreeModel * model)
{
	GtkTreeIter iter;

	gtk_tree_model_get_iter_from_string(model, &iter, path);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 2, new_text, -1);
}

void IptuxSetting::ImportNetSegment(gpointer data)
{
	IptuxSetting *ipst;
	GtkWidget *dialog;
	GtkTreeIter iter;
	NetSegment *ns;
	gchar *filename;
	GSList *list, *tmp;

	ipst = (IptuxSetting *) data, list = NULL;
	gtk_list_store_clear(GTK_LIST_STORE(ipst->ip_model));
	dialog = gtk_file_chooser_dialog_new(_("Choose the import file"),
		     GTK_WINDOW(setting), GTK_FILE_CHOOSER_ACTION_OPEN,
		     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		     GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog),
					     GTK_RESPONSE_ACCEPT);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
						    g_get_home_dir());

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		ipst->UpdateNetSegment(filename, &list, false);
		g_free(filename), tmp = list;
		while (tmp) {
			ns = (NetSegment *) tmp->data;
			gtk_list_store_append(GTK_LIST_STORE(ipst->ip_model), &iter);
			gtk_list_store_set(GTK_LIST_STORE(ipst->ip_model),
					   &iter, 0, ns->startip, 1, ns->endip,
					   2, ns->describe, -1);
			tmp = tmp->next;
		}
		g_slist_foreach(list, GFunc(remove_foreach),
				GINT_TO_POINTER(NETSEGMENT));
		g_slist_free(list);
	}
	gtk_widget_destroy(dialog);
}

void IptuxSetting::ExportNetSegment(gpointer data)
{
	IptuxSetting *ipst;
	GtkWidget *dialog;
	GtkTreeIter iter;
	gchar *filename;
	NetSegment *ns;
	GSList *list;

	ipst = (IptuxSetting *) data, list = NULL;
	dialog = gtk_file_chooser_dialog_new(_("Save the export file"),
		     GTK_WINDOW(setting), GTK_FILE_CHOOSER_ACTION_SAVE,
		     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		     GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog),
					GTK_RESPONSE_ACCEPT);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
					    g_get_home_dir());

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT &&
	    gtk_tree_model_get_iter_first(ipst->ip_model, &iter)) {
		do {
			ns = new NetSegment(NULL, NULL, NULL);
			gtk_tree_model_get(ipst->ip_model, &iter, 0,
					   &ns->startip, 1, &ns->endip, 2,
					   &ns->describe, -1);
			ns->describe = ns->describe ? ns->describe : Strdup("");
			list = g_slist_append(list, ns);
		} while (gtk_tree_model_iter_next(ipst->ip_model, &iter));
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		ipst->UpdateNetSegment(filename, &list, true);
		g_slist_foreach(list, GFunc(remove_foreach),
				GINT_TO_POINTER(NETSEGMENT));
		g_free(filename), g_slist_free(list);
	}
	gtk_widget_destroy(dialog);
}

void IptuxSetting::ClickOk(gpointer data)
{
	IptuxSetting::ClickApply(data);
	gtk_widget_destroy(setting);
}

void IptuxSetting::ClickApply(gpointer data)
{
	extern Control ctr;
	IptuxSetting *ipst;

	ipst = (IptuxSetting *) data;
	ipst->ObtainPerson();
	ipst->ObtainSystem();
#ifdef HAVE_GST
	ipst->ObtainSound();
#endif
	ipst->ObtainIpseg();
	ctr.dirty = true;
	ipst->UpdateMyInfo();
}

void IptuxSetting::SettingDestroy(gpointer data)
{
	extern Sound sound;
	extern Control ctr;

	setting = NULL;
	delete(IptuxSetting *) data;
	sound.AdjustVolume(ctr.volume);
}
