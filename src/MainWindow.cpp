//
// C++ Implementation: MainWindow
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "MainWindow.h"
#include "StatusIcon.h"
#include "DetectPal.h"
#include "Transport.h"
#include "IptuxSetting.h"
#include "ShareFile.h"
#include "DialogGroup.h"
#include "AboutIptux.h"
#include "CoreThread.h"
#include "UdpData.h"
#include "DialogPeer.h"
#include "RevisePal.h"
#include "SendFile.h"
#include "Control.h"
#include "my_entry.h"
#include "output.h"
#include "support.h"
#include "baling.h"
#include "utils.h"

const char *MainWindow::localip[] = {
	"10.0.0.0",
	"10.255.255.255",
	"172.16.0.0",
	"172.31.255.255",
	"192.168.0.0",
	"192.168.255.255",
	"Others",
	NULL
};

 MainWindow::MainWindow():window(NULL), client_paned(NULL),
tips(NULL), pal_tree(NULL), tree_model(NULL), accel(NULL)
{
}

MainWindow::~MainWindow()
{
	gtk_widget_destroy(window);
	g_object_unref(tree_model);
	g_object_unref(accel);
}

void MainWindow::InitSelf()
{
	tree_model = CreatePalTreeModel();
	InitPalTreeModel();
}

void MainWindow::CreateWindow()
{
	CreatePanel();
	CreateAllArea();
}

bool MainWindow::PalGetModelIter(gpointer data, GtkTreeIter * iter)
{
	GtkTreeIter parent;
	gpointer pal;

	Ipv4GetParent(((Pal*)data)->Ipv4Quote(), &parent);
	if (!gtk_tree_model_iter_children(tree_model, iter, &parent))
		return false;

	do {
		gtk_tree_model_get(tree_model, iter, 7, &pal, -1);
		if (pal == data)
			return true;
	} while (gtk_tree_model_iter_next(tree_model, iter));

	return false;
}

void MainWindow::AttachItemToModel(in_addr_t ipv4, GtkTreeIter * iter)
{
	GtkTreeIter parent;

	Ipv4GetParent(ipv4, &parent);
	gtk_tree_store_append(GTK_TREE_STORE(tree_model), iter, &parent);
}

void MainWindow::SetValueToModel(gpointer data, GtkTreeIter * iter)
{
	extern Control ctr;
	char buf[MAX_BUF], ipstr[INET_ADDRSTRLEN];
	GdkPixbuf *pixbuf;
	Pal *pal;

	pal = (Pal *)data;
	pixbuf = pal->GetIconPixbuf();
	inet_ntop(AF_INET, &pal->Ipv4Quote(), ipstr, INET_ADDRSTRLEN);
	snprintf(buf, MAX_BUF, "%s\n%s", pal->NameQuote(), ipstr);
	gtk_tree_store_set(GTK_TREE_STORE(tree_model), iter, 0, pixbuf,
			   1, NULL, 2, buf, 3, ctr.font, 4, TRUE, 6, FALSE,
			   7, data, 8, FALSE, -1);
	if (pixbuf)
		g_object_unref(pixbuf);
}

void MainWindow::MakeItemBlinking(GtkTreeIter * iter, bool blink)
{
	gchar *color;

	gtk_tree_model_get(tree_model, iter, 1, &color, -1);
	g_free(color);
	gtk_tree_store_set(GTK_TREE_STORE(tree_model), iter, 1,
			   (!color && blink) ? "#52B838" : NULL, -1);
}

void MainWindow::DelItemFromModel(gpointer data)
{
	GtkTreeIter iter;

	if (PalGetModelIter(data, &iter))
		gtk_tree_store_remove(GTK_TREE_STORE(tree_model), &iter);
}

void MainWindow::CreatePanel()
{
	extern Control ctr;
	extern struct interactive inter;
	GdkGeometry geometry = { 50, 200, 2000, 2000, 0, 0, 1, 10, 0.0, 0.0,
		GDK_GRAVITY_NORTH_WEST
	};
	GdkWindowHints hints =
	    GdkWindowHints(GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE |
			   GDK_HINT_BASE_SIZE | GDK_HINT_RESIZE_INC |
			   GDK_HINT_WIN_GRAVITY | GDK_HINT_USER_POS |
			   GDK_HINT_USER_SIZE);
	GdkPixbuf *pixbuf;

	inter.window = window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), _("iptux"));
	pixbuf = gdk_pixbuf_new_from_file_at_size(__LOGO_PATH "/ip-tux.png",
							25, 25, NULL);
	if (pixbuf) {
		gtk_window_set_default_icon(pixbuf);
		g_object_unref(pixbuf);
	} else
		pwarning(Fail, "%s \"" __LOGO_PATH "/ip-tux.png\" %s",
			 _("Icon file"), _("is lost!"));
	gtk_window_set_geometry_hints(GTK_WINDOW(window), window,
						      &geometry, hints);
	gtk_window_set_default_size(GTK_WINDOW(window), GINT(ctr.pix * 70),
						    GINT(ctr.pix * 150));
	accel = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(window), accel);

	g_signal_connect_swapped(window, "delete-event",
			 G_CALLBACK(StatusIcon::SwitchWindowMode), NULL);
	gtk_widget_show(window);
}

void MainWindow::CreateAllArea()
{
	GtkWidget *menu_bar;
	GtkWidget *box, *sw;

	client_paned = create_paned();
	gtk_container_add(GTK_CONTAINER(window), client_paned);
	box = create_box();
	gtk_paned_pack1(GTK_PANED(client_paned), box, true, true);
	menu_bar = CreateMenuBar();
	gtk_box_pack_start(GTK_BOX(box), menu_bar, FALSE, FALSE, 0);
	tips = create_label(_("pals online: 0"));
	gtk_box_pack_start(GTK_BOX(box), tips, FALSE, FALSE, 0);
	sw = create_scrolled_window();
	gtk_container_set_border_width(GTK_CONTAINER(sw), 4);
	gtk_box_pack_start(GTK_BOX(box), sw, TRUE, TRUE, 0);
	pal_tree = CreatePalTree();
	gtk_container_add(GTK_CONTAINER(sw), pal_tree);
}

GtkWidget *MainWindow::CreateMenuBar()
{
	GtkWidget *menu_bar;

	menu_bar = gtk_menu_bar_new();
	gtk_widget_show(menu_bar);
	CreateFileMenu(menu_bar);
	CreateToolMenu(menu_bar);
	CreateHelpMenu(menu_bar);

	return menu_bar;
}

GtkWidget *MainWindow::CreatePalTree()
{
	GtkTreeSelection *selection;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkWidget *view;

	view = gtk_tree_view_new_with_model(tree_model);
	widget_enable_dnd_uri(view);
	gtk_tree_view_set_level_indentation(GTK_TREE_VIEW(view), 10);
	gtk_tree_view_set_show_expanders(GTK_TREE_VIEW(view), FALSE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	gtk_tree_selection_set_mode(GTK_TREE_SELECTION(selection),
					    GTK_SELECTION_NONE);
	gtk_widget_show(view);

	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	renderer = gtk_cell_renderer_pixbuf_new();	//
	gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column),
							renderer, FALSE);
	g_object_set(renderer, "follow-state", TRUE, NULL);
	gtk_tree_view_column_set_attributes(GTK_TREE_VIEW_COLUMN(column),
			    renderer, "pixbuf", 0, "cell-background", 1, NULL);
	renderer = gtk_cell_renderer_text_new();	//
	gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column),
							renderer, TRUE);
	g_object_set(renderer, "xalign", 0.0, "wrap-mode", PANGO_WRAP_WORD,
					     "foreground", "#52B838", NULL);
	gtk_tree_view_column_set_attributes(GTK_TREE_VIEW_COLUMN(column),
			    renderer, "text", 2, "font", 3, "visible", 4, NULL);
	renderer = gtk_cell_renderer_text_new();	//
	gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column),
							renderer, TRUE);
	g_object_set(renderer, "xalign", 0.0, "wrap-mode", PANGO_WRAP_WORD,
					     "foreground", "#52B838", NULL);
	gtk_tree_view_column_set_attributes(GTK_TREE_VIEW_COLUMN(column),
				    renderer, "markup", 5, "visible", 6, NULL);

	g_object_set(view, "has-tooltip", TRUE, NULL);
	g_signal_connect(view, "query-tooltip",
			 G_CALLBACK(TreeQueryTooltip), tree_model);
	g_signal_connect(view, "row-activated",
			 G_CALLBACK(TreeItemActivated), tree_model);
	g_signal_connect(view, "drag-data-received",
			 G_CALLBACK(TreeDragDataReceived), tree_model);
	g_signal_connect(view, "button-release-event",
			 G_CALLBACK(TreeChangeStatus), this);
	g_signal_connect(view, "button-press-event",
			 G_CALLBACK(TreePopupMenu), this);

	return view;
}

//Panel 9,0 pixbuf,1 color,2 nickname,3 font,4 visible,5 markup,6 visible,7 data/last,8 expend
GtkTreeModel *MainWindow::CreatePalTreeModel()
{
	GtkTreeStore *model;

	model = gtk_tree_store_new(9, GDK_TYPE_PIXBUF, G_TYPE_STRING,
				   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN,
				   G_TYPE_STRING, G_TYPE_BOOLEAN,
				   G_TYPE_POINTER, G_TYPE_BOOLEAN);

	return GTK_TREE_MODEL(model);
}

void MainWindow::InitPalTreeModel()
{
	gchar ipstr[32], buf[MAX_BUF];
	GtkTreeIter iter;
	GdkPixbuf *pixbuf;
	uint8_t count;

	pixbuf = gdk_pixbuf_new_from_file(__TIP_PATH "/hide.png", NULL);
	count = 0;
	while (localip[count << 1]) {
		if (localip[(count << 1) + 1])
			snprintf(ipstr, 32, "%s~%s", localip[count << 1],
						 localip[(count << 1) + 1]);
		else
			snprintf(ipstr, 32, "%s", localip[count << 1]);
		snprintf(buf, MAX_BUF,
			 "<span style=\"italic\" underline=\"single\" size=\"small\" "
			 "foreground=\"#52B838\" weight=\"bold\">%s</span>",
			 ipstr);
		gtk_tree_store_append(GTK_TREE_STORE(tree_model), &iter, NULL);
		gtk_tree_store_set(GTK_TREE_STORE(tree_model), &iter, 0, pixbuf,
				   1, NULL, 4, FALSE, 5, buf, 6, TRUE, 7, NULL,
				   8, FALSE, -1);
		if (!localip[(count << 1) + 1])
			break;
		count++;
	}
}

void MainWindow::Ipv4GetParent(in_addr_t ipv4, GtkTreeIter * parent)
{
	in_addr_t ip1, ip2;
	uint8_t count;

	gtk_tree_model_get_iter_first(tree_model, parent);
	count = 0, ipv4 = ntohl(ipv4);
	do {
		if (!localip[count << 1] || !localip[(count << 1) + 1])
			break;
		inet_pton(AF_INET, localip[count << 1], &ip1);
		ip1 = ntohl(ip1);
		inet_pton(AF_INET, localip[(count << 1) + 1], &ip2);
		ip2 = ntohl(ip2);
		ipv4_order(&ip1, &ip2);
		if (ip1 <= ipv4 && ip2 >= ipv4)
			break;
		count++;
	} while (gtk_tree_model_iter_next(tree_model, parent));
}

void MainWindow::CreateFileMenu(GtkWidget * menu_bar)
{
	GtkWidget *image;
	GtkWidget *menu;
	GtkWidget *menu_item;

	menu_item = gtk_menu_item_new_with_mnemonic(_("_File"));
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

	menu = gtk_menu_new();
	gtk_widget_show(menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Detect"));
	image = gtk_image_new_from_file(__MENU_PATH "/detect.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(DetectPal::DetectEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Find"));
	image = gtk_image_new_from_stock(GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(AttachFindArea), this);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_separator_menu_item_new();
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Quit"));
	image = gtk_image_new_from_stock(GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(iptux_gui_quit), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
}

void MainWindow::CreateToolMenu(GtkWidget * menu_bar)
{
	GtkWidget *image;
	GtkWidget *menu;
	GtkWidget *menu_item;

	menu_item = gtk_menu_item_new_with_mnemonic(_("_Tools"));
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

	menu = gtk_menu_new();
	gtk_widget_show(menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

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
	image = gtk_image_new_from_file(__MENU_PATH "/share.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(ShareFile::ShareEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Group"));
	image = gtk_image_new_from_file(__MENU_PATH "/group.png");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(DialogGroup::DialogEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Update"));
	image = gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect_swapped(menu_item, "activate",
			 G_CALLBACK(UpdatePalTree), this);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
}

void MainWindow::CreateHelpMenu(GtkWidget * menu_bar)
{
	GtkWidget *menu;
	GtkWidget *menu_item;

	menu_item = gtk_menu_item_new_with_mnemonic(_("_Help"));
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

	menu = gtk_menu_new();
	gtk_widget_show(menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

	menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, accel);
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(AboutIptux::AboutEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_More"));
	g_signal_connect(menu_item, "activate",
			 G_CALLBACK(AboutIptux::MoreEntry), NULL);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
}

void MainWindow::UpdateTips()
{
	extern MainWindow *mwp;
	extern UdpData udt;
	char buf[MAX_BUF];
	uint32_t sum;
	GSList *tmp;

	pthread_mutex_lock(udt.MutexQuote());
	sum = 0, tmp = udt.PallistQuote();
	while (tmp) {
		if (FLAG_ISSET(((Pal *) tmp->data)->FlagsQuote(), 1))
			sum++;
		tmp = tmp->next;
	}
	pthread_mutex_unlock(udt.MutexQuote());
	snprintf(buf, MAX_BUF, _("pals online: %" PRIu32), sum);
	gtk_label_set_text(GTK_LABEL(mwp->tips), buf);
}

GtkWidget *MainWindow::CreatePopupPalMenu(gpointer data)
{
	GtkWidget *menu, *menu_item;

	menu = gtk_menu_new();
	gtk_widget_show(menu);

	menu_item = gtk_menu_item_new_with_label(_("Send Message"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(DialogPeer::DialogEntry), data);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Send File"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(SendFile::SendRegular), data);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Send Folder"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(SendFile::SendFolder), data);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Ask For Shared Files"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(DialogPeer::AskSharedFiles), data);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Change Information"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(RevisePal::ReviseEntry), data);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Delete Pal"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(DeletePal), data);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	return menu;
}

GtkWidget *MainWindow::CreatePopupSectionMenu()
{
	GtkWidget *menu, *submenu, *menu_item;
	GtkTreeModel *model;
	gboolean expend;

	menu = gtk_menu_new();
	gtk_widget_show(menu);

	/************************************/
	menu_item = gtk_menu_item_new_with_label(_("Sort"));
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	submenu = gtk_menu_new();
	gtk_widget_show(submenu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), submenu);

	menu_item = gtk_menu_item_new_with_label(_("By IP"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(SortSectionByIP), this);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("By Nickname"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(SortSectionByNickname), this);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("By Group"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(SortSectionByGroup), this);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("By Communication"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(SortSectionByCommunication), this);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menu_item);

	/************************************/
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(pal_tree));
	gtk_tree_model_get(model, &opt_iter, 8, &expend, -1);
	if (expend)
		menu_item = gtk_menu_item_new_with_label(_("Collapse"));
	else
		menu_item = gtk_menu_item_new_with_label(_("Expand"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(TreeItemChangeStatus), this);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	return menu;
}

//list_model 7,0 pixbuf,1 name,2 group,3 ipstr,4 user,5 host,6 pal
GtkTreeModel *MainWindow::CreatePalListModel()
{
	GtkListStore *model;

	model = gtk_list_store_new(7, GDK_TYPE_PIXBUF,
				   G_TYPE_STRING, G_TYPE_STRING,
				   G_TYPE_STRING, G_TYPE_STRING,
				   G_TYPE_STRING, G_TYPE_POINTER);

	return GTK_TREE_MODEL(model);
}

GtkWidget *MainWindow::CreatePalListView()
{
	GtkWidget *view;
	GtkTreeModel *model;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkTreeSelection *selection;

	model = CreatePalListModel();
	view = gtk_tree_view_new_with_model(model);
	widget_enable_dnd_uri(view);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	gtk_tree_selection_set_mode(GTK_TREE_SELECTION(selection),
				    GTK_SELECTION_NONE);
	gtk_widget_show(view);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("nickname"));
	renderer = gtk_cell_renderer_pixbuf_new();
	g_object_set(renderer, "follow-state", TRUE, NULL);
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "pixbuf", 0, NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("group"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("IPv4"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 3, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("user"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 4, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("host"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 5, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	g_signal_connect(view, "row-activated",
			 G_CALLBACK(ListItemActivated), model);
	g_signal_connect(view, "drag-data-received",
			 G_CALLBACK(ListDragDataReceived), model);
	g_signal_connect(view, "button-press-event",
			 G_CALLBACK(ListPopupMenu), this);

	return view;
}

void MainWindow::UpdatePalTree(gpointer data)
{
	extern UdpData udt;
	MainWindow *mw;
	GSList *tmp;

	pthread_mutex_lock(udt.MutexQuote());
	tmp = udt.PallistQuote();
	while (tmp) {
		FLAG_CLR(((Pal *) tmp->data)->FlagsQuote(), 1);
		tmp = tmp->next;
	}
	g_queue_clear(udt.MsgqueueQuote());
	pthread_mutex_unlock(udt.MutexQuote());

	mw = (MainWindow *)data;
	gtk_tree_store_clear(GTK_TREE_STORE(mw->tree_model));
	mw->InitPalTreeModel();

	thread_create(ThreadFunc(CoreThread::NotifyAll), NULL, false);
}

void MainWindow::DeletePal(gpointer data)
{
	extern UdpData udt;
	extern MainWindow *mwp;
	GtkTreeIter iter;
	GList *tmp;
	Pal *pal;

	pal = (Pal *) data;
	if (!udt.Ipv4GetPalPos(pal->Ipv4Quote()))
		return;

	if (mwp->PalGetModelIter(pal, &iter))
		gtk_tree_store_remove(GTK_TREE_STORE(mwp->tree_model), &iter);
	tmp = (GList *) udt.PalGetMsgPos(pal);
	if (tmp) {
		pthread_mutex_lock(udt.MutexQuote());
		g_queue_delete_link(udt.MsgqueueQuote(), tmp);
		pthread_mutex_unlock(udt.MutexQuote());
	}
	FLAG_CLR(pal->FlagsQuote(), 1);
	FLAG_SET(pal->FlagsQuote(), 3);
}

gboolean MainWindow::TreeQueryTooltip(GtkWidget * view, gint x, gint y,
				      gboolean key, GtkTooltip * tooltip,
				      GtkTreeModel * model)
{
	extern Control ctr;
	GdkColor color = { 8, 65535, 65535, 55000 };
	GtkWidget *text_view;
	GtkTextBuffer *buffer;
	GtkTreePath *path;
	GtkTreeIter iter;
	Pal *pal;

	if (key || !gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(view),
					  x, y, &path, NULL, NULL, NULL))
		return FALSE;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	gtk_tree_model_get(model, &iter, 7, &pal, -1);
	if (!pal)
		return FALSE;

	text_view = create_text_view();
	gtk_widget_modify_base(text_view, GTK_STATE_NORMAL, &color);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_NONE);
	buffer = gtk_text_buffer_new(ctr.table);
	DialogPeer::FillPalInfoToBuffer(pal, buffer, false);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(text_view), buffer);
	gtk_tooltip_set_custom(tooltip, text_view);
	g_signal_connect_swapped(text_view, "destroy",
				 G_CALLBACK(g_object_unref), buffer);

	return TRUE;
}

void MainWindow::TreeItemActivated(GtkWidget * view, GtkTreePath * path,
				   GtkTreeViewColumn * column,
				   GtkTreeModel * model)
{
	GtkTreeIter iter;
	gpointer data;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, 7, &data, -1);
	if (data)
		DialogPeer::DialogEntry(data);
}

gboolean MainWindow::TreePopupMenu(GtkWidget * view, GdkEventButton * event,
				   gpointer data)
{
	MainWindow *mw;
	GtkTreeModel *model;
	GtkTreePath *path;
	Pal *pal;

	if (event->button != 3
	    || !gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(view),
		      GINT(event->x), GINT(event->y), &path, NULL, NULL, NULL))
		return FALSE;

	mw = (MainWindow *) data;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
	gtk_tree_model_get_iter(model, &mw->opt_iter, path);
	gtk_tree_path_free(path);
	gtk_tree_model_get(model, &mw->opt_iter, 7, &pal, -1);
	if (pal)
		gtk_menu_popup(GTK_MENU(mw->CreatePopupPalMenu(pal)),
			   NULL, NULL, NULL, NULL, event->button, event->time);
	else
		gtk_menu_popup(
		    GTK_MENU(mw->CreatePopupSectionMenu()),
		    NULL, NULL, NULL, NULL, event->button, event->time);

	return TRUE;
}

gboolean MainWindow::TreeChangeStatus(GtkWidget * view, GdkEventButton * event,
				      gpointer data)
{
	MainWindow *mw;
	GtkTreePath *path;
	GtkTreeModel *model;

	if (event->button != 1
		   || !gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(view),
		      GINT(event->x), GINT(event->y), &path, NULL, NULL, NULL))
		return FALSE;

	if (gtk_tree_path_get_depth(path) == 1) {
		mw = (MainWindow *) data;
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(mw->pal_tree));
		gtk_tree_model_get_iter(model, &mw->opt_iter, path);
		TreeItemChangeStatus(mw);
	}
	gtk_tree_path_free(path);
	return FALSE;
}

void MainWindow::TreeItemChangeStatus(gpointer data)
{
	MainWindow *mw;
	GtkTreeModel *model;
	GtkTreePath *path;
	GdkPixbuf *pixbuf;
	gboolean expend;

	mw = (MainWindow *) data;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(mw->pal_tree));
	path = gtk_tree_model_get_path(model, &mw->opt_iter);
	gtk_tree_model_get(model, &mw->opt_iter, 8, &expend, -1);
	if (expend) {
		pixbuf = gdk_pixbuf_new_from_file(__TIP_PATH "/hide.png", NULL);
		gtk_tree_store_set(GTK_TREE_STORE(model), &mw->opt_iter, 0,
						   pixbuf, 8, FALSE, -1);
		gtk_tree_view_collapse_row(GTK_TREE_VIEW(mw->pal_tree), path);
	} else {
		pixbuf = gdk_pixbuf_new_from_file(__TIP_PATH "/show.png", NULL);
		gtk_tree_store_set(GTK_TREE_STORE(model), &mw->opt_iter, 0,
						   pixbuf, 8, TRUE, -1);
		gtk_tree_view_expand_row(GTK_TREE_VIEW(mw->pal_tree), path,
								 FALSE);
	}
	if (pixbuf)
		g_object_unref(pixbuf);
	gtk_tree_path_free(path);
}

void MainWindow::TreeDragDataReceived(GtkWidget * view,
				      GdkDragContext * context, gint x, gint y,
				      GtkSelectionData * select, guint info,
				      guint time, GtkTreeModel * model)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	gpointer data;

	if (!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(view),
					   x, y, &path, NULL, NULL, NULL))
		return;
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	gtk_tree_model_get(model, &iter, 7, &data, -1);
	if (data)
		DialogPeer::DragDataReceived(data, context, x, y,
					     select, info, time);
}

void MainWindow::SortSectionByIP(gpointer data)
{
	MainWindow *mw;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gint *table, count, count1, count2, sum;
	in_addr_t *ipv4, tmp;
	Pal *pal;

	mw = (MainWindow *) data;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(mw->pal_tree));
	if ((sum = gtk_tree_model_iter_n_children(model, &mw->opt_iter)) < 2)
		return;

	table = (gint *) Malloc(sum * sizeof(gint));
	ipv4 = (in_addr_t *) Malloc(sum * sizeof(in_addr_t));
	gtk_tree_model_iter_children(model, &iter, &mw->opt_iter);
	count = 0;
	do {
		gtk_tree_model_get(model, &iter, 7, &pal, -1);
		*(ipv4 + count) = ntohl(pal->Ipv4Quote());
		*(table + count) = count;
		count++;
	} while (gtk_tree_model_iter_next(model, &iter));

	count1 = 0, sum--;
	while (count1 < sum) {
		count2 = count1 + 1;
		while (count2 <= sum) {
			if (*(ipv4 + count1) > *(ipv4 + count2)) {
				tmp = *(ipv4 + count1);
				count = *(table + count1);
				*(ipv4 + count1) = *(ipv4 + count2);
				*(table + count1) = *(table + count2);
				*(ipv4 + count2) = tmp;
				*(table + count2) = count;
			}
			count2++;
		}
		count1++;
	}

	gtk_tree_store_reorder(GTK_TREE_STORE(model), &mw->opt_iter, table);
	free(table), free(ipv4);
}

void MainWindow::SortSectionByNickname(gpointer data)
{
	MainWindow *mw;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gint *table, count, count1, count2, sum;
	const char **name, *tmp;
	Pal *pal;

	mw = (MainWindow *) data;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(mw->pal_tree));
	if ((sum = gtk_tree_model_iter_n_children(model, &mw->opt_iter)) < 2)
		return;

	table = (gint *) Malloc(sum * sizeof(gint));
	name = (const char **)Malloc(sum * sizeof(pointer));
	gtk_tree_model_iter_children(model, &iter, &mw->opt_iter);
	count = 0;
	do {
		gtk_tree_model_get(model, &iter, 7, &pal, -1);
		*(name + count) = pal->NameQuote();
		*(table + count) = count;
		count++;
	} while (gtk_tree_model_iter_next(model, &iter));

	count1 = 0, sum--;
	while (count1 < sum) {
		count2 = count1 + 1;
		while (count2 <= sum) {
			if (strcmp(*(name + count1), *(name + count2)) > 0) {
				tmp = *(name + count1);
				count = *(table + count1);
				*(name + count1) = *(name + count2);
				*(table + count1) = *(table + count2);
				*(name + count2) = tmp;
				*(table + count2) = count;
			}
			count2++;
		}
		count1++;
	}

	gtk_tree_store_reorder(GTK_TREE_STORE(model), &mw->opt_iter, table);
	free(table), free(name);
}

void MainWindow::SortSectionByGroup(gpointer data)
{
	MainWindow *mw;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gint *table, count, count1, count2, sum;
	const char **group, *tmp;
	Pal *pal;

	mw = (MainWindow *) data;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(mw->pal_tree));
	if ((sum = gtk_tree_model_iter_n_children(model, &mw->opt_iter)) < 2)
		return;

	table = (gint *) Malloc(sum * sizeof(gint));
	group = (const char **)Malloc(sum * sizeof(pointer));
	gtk_tree_model_iter_children(model, &iter, &mw->opt_iter);
	count = 0;
	do {
		gtk_tree_model_get(model, &iter, 7, &pal, -1);
		*(group + count) = pal->GroupQuote();
		*(table + count) = count;
		count++;
	} while (gtk_tree_model_iter_next(model, &iter));

	count1 = 0, sum--;
	while (count1 < sum) {
		count2 = count1 + 1;
		while (count2 <= sum) {
			if (strcmp(*(group + count1), *(group + count2)) > 0) {
				tmp = *(group + count1);
				count = *(table + count1);
				*(group + count1) = *(group + count2);
				*(table + count1) = *(table + count2);
				*(group + count2) = tmp;
				*(table + count2) = count;
			}
			count2++;
		}
		count1++;
	}

	gtk_tree_store_reorder(GTK_TREE_STORE(model), &mw->opt_iter, table);
	free(table), free(group);
}

void MainWindow::SortSectionByCommunication(gpointer data)
{
	MainWindow *mw;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gint *table, count, count1, count2, sum;
	uint32_t *packetno, tmp;
	Pal *pal;

	mw = (MainWindow *) data;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(mw->pal_tree));
	if ((sum = gtk_tree_model_iter_n_children(model, &mw->opt_iter)) < 2)
		return;

	table = (gint *) Malloc(sum * sizeof(gint));
	packetno = (uint32_t *) Malloc(sum * sizeof(uint32_t));
	gtk_tree_model_iter_children(model, &iter, &mw->opt_iter);
	count = 0;
	do {
		gtk_tree_model_get(model, &iter, 7, &pal, -1);
		*(packetno + count) = pal->PacketnQuote();
		*(table + count) = count;
		count++;
	} while (gtk_tree_model_iter_next(model, &iter));

	count1 = 0, sum--;
	while (count1 < sum) {
		count2 = count1 + 1;
		while (count2 <= sum) {
			if (*(packetno + count1) < *(packetno + count2)) {
				tmp = *(packetno + count1);
				count = *(table + count1);
				*(packetno + count1) = *(packetno + count2);
				*(table + count1) = *(table + count2);
				*(packetno + count2) = tmp;
				*(table + count2) = count;
			}
			count2++;
		}
		count1++;
	}

	gtk_tree_store_reorder(GTK_TREE_STORE(model), &mw->opt_iter, table);
	free(table), free(packetno);
}

void MainWindow::AttachFindArea(gpointer data)
{
	MainWindow *mw;
	GtkWidget *sw, *view, *button, *image, *entry;
	GtkWidget *box, *hbox;

	mw = (MainWindow *) data;
	if (gtk_paned_get_child2(GTK_PANED(mw->client_paned)))
		return;
	box = create_box();
	gtk_paned_pack2(GTK_PANED(mw->client_paned), box, TRUE, TRUE);

	sw = create_scrolled_window();
	gtk_box_pack_start(GTK_BOX(box), sw, TRUE, TRUE, 0);
	view = mw->CreatePalListView();
	gtk_container_add(GTK_CONTAINER(sw), view);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
	button = gtk_button_new();
	g_object_set(button, "relief", GTK_RELIEF_NONE, NULL);
	image = gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
	gtk_button_set_image(GTK_BUTTON(button), image);
	gtk_widget_show(button);
	g_signal_connect_swapped(button, "clicked", G_CALLBACK(g_object_unref),
				 gtk_tree_view_get_model(GTK_TREE_VIEW(view)));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(gtk_widget_destroy), box);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	entry = my_entry::create_entry(NULL, _("search pals"));
	gtk_widget_add_events(entry, GDK_KEY_PRESS_MASK);
	g_signal_connect(entry, "key-press-event",
			 G_CALLBACK(ClearFindEntry), NULL);
	g_signal_connect(entry, "changed", G_CALLBACK(FindEntryChanged), view);
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);

	FindEntryChanged(entry, view);
	gtk_widget_grab_focus(entry);
}

gboolean MainWindow::ClearFindEntry(GtkWidget * entry, GdkEventKey * event)
{
	if (event->keyval != GDK_Escape)
		return FALSE;
	gtk_entry_set_text(GTK_ENTRY(entry), "");
	return TRUE;
}

void MainWindow::FindEntryChanged(GtkWidget * entry, GtkWidget * view)
{
	extern UdpData udt;
	char ipstr[INET_ADDRSTRLEN];
	const gchar *text;
	GdkPixbuf *pixbuf;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GSList *tmp;
	Pal *pal;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
	gtk_list_store_clear(GTK_LIST_STORE(model));
	text = gtk_entry_get_text(GTK_ENTRY(entry));
	pthread_mutex_lock(udt.MutexQuote());
	tmp = udt.PallistQuote();
	while (tmp) {
		pal = (Pal *) tmp->data;
		tmp = tmp->next;
		inet_ntop(AF_INET, &pal->Ipv4Quote(), ipstr, INET_ADDRSTRLEN);
		if (FLAG_ISSET(pal->FlagsQuote(), 1) && (*text == '\0'
					  || strstr(pal->NameQuote(), text)
					  || strstr(pal->GroupQuote(), text)
					  || strstr(ipstr, text)
					  || strstr(pal->UserQuote(), text)
					  || strstr(pal->HostQuote(), text))) {
			pixbuf = pal->GetIconPixbuf();
			gtk_list_store_append(GTK_LIST_STORE(model), &iter);
			gtk_list_store_set(GTK_LIST_STORE(model), &iter,
					   0, pixbuf, 1, pal->NameQuote(),
					   2, pal->GroupQuote(), 3, ipstr,
					   4, pal->UserQuote(),
					   5, pal->HostQuote(), 6, pal, -1);
			if (pixbuf)
				g_object_unref(pixbuf);
		}
	}
	pthread_mutex_unlock(udt.MutexQuote());
	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(view));
}

void MainWindow::ListItemActivated(GtkWidget * view, GtkTreePath * path,
				   GtkTreeViewColumn * column,
				   GtkTreeModel * model)
{
	GtkTreeIter iter;
	gpointer data;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, 6, &data, -1);
	DialogPeer::DialogEntry(data);
}

gboolean MainWindow::ListPopupMenu(GtkWidget * view, GdkEventButton * event,
				   gpointer data)
{
	MainWindow *mw;
	GtkTreePath *path;
	GtkTreeModel *model;
	GtkTreeIter iter;
	Pal *pal;

	if (event->button != 3
	    || !gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(view),
					      GINT(event->x), GINT(event->y),
					      &path, NULL, NULL, NULL))
		return FALSE;

	mw = (MainWindow *) data;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	gtk_tree_model_get(model, &iter, 6, &pal, -1);
	gtk_menu_popup(GTK_MENU(mw->CreatePopupPalMenu(pal)), NULL, NULL,
			       NULL, NULL, event->button, event->time);

	return TRUE;
}

void MainWindow::ListDragDataReceived(GtkWidget * view,
				      GdkDragContext * context, gint x, gint y,
				      GtkSelectionData * select, guint info,
				      guint time, GtkTreeModel * model)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	gpointer data;

	if (!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(view),
					   x, y, &path, NULL, NULL, NULL))
		return;
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	gtk_tree_model_get(model, &iter, 6, &data, -1);
	DialogPeer::DragDataReceived(data, context, x, y, select, info, time);
}
