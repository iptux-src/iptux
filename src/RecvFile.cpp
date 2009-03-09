//
// C++ Implementation: RecvFile
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "RecvFile.h"
#include "DialogGroup.h"
#include "Transport.h"
#include "IptuxSetting.h"
#include "Control.h"
#include "Log.h"
#include "utils.h"
#include "baling.h"

 RecvFile::RecvFile(gpointer data):filelist(NULL), file_model(NULL)
{
	pal = (Pal *) ((struct recvfile_para *)data)->data;
	msg = ((struct recvfile_para *)data)->msg;
	commandn = ((struct recvfile_para *)data)->commandn;
	packetn = ((struct recvfile_para *)data)->packetn;
	free(data);
}

RecvFile::~RecvFile()
{
	free(msg);
	g_slist_foreach(filelist, GFunc(remove_foreach),
				GINT_TO_POINTER(FILEINFO));
	g_slist_free(filelist);
//      g_object_unref(file_model); //他处释放
}

void RecvFile::RecvEntry(gpointer data)
{
	RecvFile rf(data);

	rf.ParseExtra();
	gdk_threads_enter();
	rf.CreateRecvWindow();
	gdk_threads_leave();
}

void RecvFile::ParseExtra()
{
	char *ptr;

	ptr = msg;
	while (ptr && *ptr)
		filelist = g_slist_append(filelist, DivideFileinfo(&ptr));
}

void RecvFile::CreateRecvWindow()
{
	GtkWidget *window, *box, *sw, *view;
	GtkWidget *hbox, *chooser, *label, *hbb, *button;
	GtkTreeSelection *selection;

	file_model = CreateRecvModel();
	if (commandn & IPTUX_SHAREDOPT)
		window = create_window(_("Pal's shared files"), 135, 85);
	else
		window = create_window(_("File receive management"), 135, 85);
	gtk_container_set_border_width(GTK_CONTAINER(window), 5);
	g_signal_connect_swapped(window, "destroy",
				 G_CALLBACK(g_object_unref), file_model);
	box = create_box();
	gtk_container_add(GTK_CONTAINER(window), box);

	sw = create_scrolled_window();
	gtk_box_pack_start(GTK_BOX(box), sw, TRUE, TRUE, 0);
	view = CreateRecvView();
	gtk_container_add(GTK_CONTAINER(sw), view);

	hbox = create_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
	label = create_label(_("Save To: "));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	chooser = IptuxSetting::CreateArchiveChooser();
	gtk_box_pack_start(GTK_BOX(hbox), chooser, FALSE, FALSE, 0);
	label = create_label("");
	gtk_box_pack_end(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	g_signal_connect(selection, "changed",
				 G_CALLBACK(SelectItemChanged), chooser);
	g_signal_connect(chooser, "current-folder-changed",
				 G_CALLBACK(ChooserResetLabel), label);
	g_signal_connect(chooser, "current-folder-changed",
			 G_CALLBACK(ChooserResetModel), selection);

	label = gtk_hseparator_new();
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);

	hbb = create_button_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbb, FALSE, FALSE, 0);
	button = create_button(_("Receive"));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(AdditionRecvFile), file_model);
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(gtk_widget_destroy), window);
	gtk_box_pack_start(GTK_BOX(hbb), button, FALSE, FALSE, 0);
	button = create_button(_("Refuse"));
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(gtk_widget_destroy), window);
	gtk_box_pack_start(GTK_BOX(hbb), button, FALSE, FALSE, 0);
}

gpointer RecvFile::DivideFileinfo(char **ptr)
{
	FileInfo *file;

	file = new FileInfo(iptux_get_dec_number(*ptr, 0),
			    ipmsg_get_filename(*ptr, 1),
			    iptux_get_hex64_number(*ptr, 2),
			    iptux_get_hex_number(*ptr, 4));

	//分割，格式1 (\a) 格式2(:\a) 格式3(\a:) 格式4(:\a:)
	*ptr = strchr(*ptr, '\a');
	if (*ptr)
		(*ptr)++;
	if (*ptr && (**ptr == ':'))
		(*ptr)++;

	return file;
}

//11,0 bool,1 filename,2 owner,3 size,4 type,5 path,6 packetn,7 fileid,8 size,9 type,10 data
GtkTreeModel *RecvFile::CreateRecvModel()
{
	extern Control ctr;
	GtkListStore *model;
	GtkTreeIter iter;
	FileInfo *file;
	GSList *tmp;
	char *ptr;

	model = gtk_list_store_new(11, G_TYPE_BOOLEAN, G_TYPE_STRING,
				   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
				   G_TYPE_STRING, G_TYPE_UINT, G_TYPE_UINT,
				   G_TYPE_UINT64, G_TYPE_UINT, G_TYPE_POINTER);
	tmp = filelist;
	while (tmp) {
		file = (FileInfo *) tmp->data;
		ptr = number_to_string_size(file->filesize);
		gtk_list_store_append(model, &iter);
		gtk_list_store_set(model, &iter, 0, TRUE, 1, file->filename,
				   2, pal->NameQuote(), 3, ptr, 5, ctr.path,
				   6, packetn, 7, file->fileid, 8, file->filesize,
				   9, file->fileattr, 10, pal, -1);
		free(ptr);
		if (GET_MODE(file->fileattr) == IPMSG_FILE_REGULAR)
			gtk_list_store_set(model, &iter, 4, _("regular"), -1);
		else if (GET_MODE(file->fileattr) == IPMSG_FILE_DIR)
			gtk_list_store_set(model, &iter, 4, _("directory"), -1);
		else
			gtk_list_store_set(model, &iter, 4, _("unknown"), -1);
		tmp = tmp->next;
	}

	return GTK_TREE_MODEL(model);
}

GtkWidget *RecvFile::CreateRecvView()
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkWidget *view;
	GtkTreeSelection *selection;

	view = gtk_tree_view_new_with_model(file_model);
	g_signal_connect_swapped(view, "button-press-event",
			 G_CALLBACK(DialogGroup::PopupPickMenu), file_model);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
	gtk_widget_show(view);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("receive"));
	renderer = gtk_cell_renderer_toggle_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer,
					    "active", 0, NULL);
	g_signal_connect_swapped(renderer, "toggled",
			 G_CALLBACK(DialogGroup::ViewToggleChange), file_model);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("filename"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 1, NULL);
	g_object_set(renderer, "editable", TRUE, NULL);
	g_signal_connect(renderer, "edited",
			 G_CALLBACK(CellEditText), file_model);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("belong"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("length"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 3, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("type"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 4, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

	return view;
}

void RecvFile::CellEditText(GtkCellRendererText * renderer, gchar * path,
			    gchar * new_text, GtkTreeModel * model)
{
	GtkTreeIter iter;

	gtk_tree_model_get_iter_from_string(model, &iter, path);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 1, new_text, -1);
}

void RecvFile::SelectItemChanged(GtkTreeSelection *selection, GtkWidget *chooser)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *path;

	gtk_tree_selection_get_selected(selection, &model, &iter);
	gtk_tree_model_get(model, &iter, 5, &path, -1);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), path);
	g_free(path);
}

void RecvFile::ChooserResetLabel(GtkWidget *chooser, GtkWidget *label)
{
	uint64_t avail, total;
	char buf[MAX_BUF], *path, *str_avail, *str_total;

	path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
	get_file_system_info(path, &avail, &total);
	g_free(path);
	str_avail = number_to_string_size(avail);
	str_total = number_to_string_size(total);
	snprintf(buf, MAX_BUF, _("Free:%s Total:%s"), str_avail, str_total);
	free(str_avail), free(str_total);
	gtk_label_set_label(GTK_LABEL(label), buf);
}

void RecvFile::ChooserResetModel(GtkWidget *chooser, GtkTreeSelection *selection)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *path;

	path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
	if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
		if (gtk_tree_model_get_iter_first(model, &iter)) {
			do {
				gtk_list_store_set(GTK_LIST_STORE(model), &iter,
								  5, path, -1);
			}  while (gtk_tree_model_iter_next(model, &iter));
		}
	} else
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, 5, path, -1);
	g_free(path);
}

void RecvFile::AdditionRecvFile(GtkTreeModel * model)
{
	extern Log mylog;
	extern Transport trans;
	uint64_t filesize;
	uint32_t packetn, fileid, fileattr;
	gchar *filename, *filestr, *path;
	GtkTreeIter iter1, iter2, *iter;
	GdkPixbuf *pixbuf;
	gboolean active;
	Pal *pal;

	if (!gtk_tree_model_get_iter_first(model, &iter1))
		return;
	pixbuf = gdk_pixbuf_new_from_file(__TIP_PATH "/recv.png", NULL);
	do {
		gtk_tree_model_get(model, &iter1, 0, &active, 1, &filename,
				3, &filestr, 5, &path, 6, &packetn, 7, &fileid,
				8, &filesize, 9, &fileattr, 10, &pal, -1);
		if (!active) {
			g_free(filename), g_free(filestr), g_free(path);
			continue;
		}

		gtk_list_store_append(GTK_LIST_STORE(trans.TransModelQuote()),
							      &iter2);
		gtk_list_store_set(GTK_LIST_STORE(trans.TransModelQuote()),
				   &iter2, 0, pixbuf, 1, _("receive"),
				   2, filename, 3, pal->NameQuote(), 4, "0B",
				   5, filestr, 6, "0B/s", 7, 0, 8, 0,
				   9, packetn, 10, fileid, 11, filesize,
				   12, fileattr, 13, path, 14, pal, -1);
		g_free(filename), g_free(filestr), g_free(path);

		iter = gtk_tree_iter_copy(&iter2);
		thread_create(ThreadFunc(Transport::RecvFileEntry), iter, false);
	} while (gtk_tree_model_iter_next(model, &iter1));
	if (pixbuf)
		g_object_unref(pixbuf);
	mylog.SystemLog(_("Begin receiving paper!"));
}
