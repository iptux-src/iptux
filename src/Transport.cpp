//
// C++ Implementation: Transport
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Transport.h"
#include "Command.h"
#include "Pal.h"
#include "my_file.h"
#include "baling.h"
#include "utils.h"

 Transport::Transport():transport(NULL), trans_view(NULL), trans_model(NULL),
flag(0)
{
}

Transport::~Transport()
{
	if (transport)
		gtk_widget_destroy(transport);
	g_object_unref(trans_model);
}

void Transport::InitSelf()
{
	trans_model = CreateTransModel();
}

bool Transport::TransportActive()
{
	GtkTreeIter iter;
	gint terminate;

	if (!gtk_tree_model_get_iter_first(trans_model, &iter))
		return false;
	do {
		gtk_tree_model_get(trans_model, &iter, 8, &terminate, -1);
		if (terminate == 0)	//Runing(0)
			return true;
	} while (gtk_tree_model_iter_next(trans_model, &iter));
	return false;
}

void Transport::TransportEntry()
{
	extern Transport trans;

	if (trans.CheckExist())
		return;
	trans.CreateTransView();
	trans.CreateTransDialog();
}

void Transport::RecvFileEntry(GtkTreeIter * iter)
{
	extern Transport trans;
	uint32_t fileattr;
	gboolean result;

	gdk_threads_enter();
	gtk_tree_model_get(trans.trans_model, iter, 12, &fileattr, -1);
	TransportEntry();
	gdk_threads_leave();
	switch (GET_MODE(fileattr)) {
	case IPMSG_FILE_REGULAR:
		trans.RecvFileData(iter);
		break;
	case IPMSG_FILE_DIR:
		trans.RecvDirFiles(iter);
		break;
	default:
		break;
	}
	gtk_tree_iter_free(iter);
}

void Transport::SendFileEntry(int sock, GtkTreeIter * iter, uint32_t fileattr)
{
	extern Transport trans;

	gdk_threads_enter();
	TransportEntry();
	gdk_threads_leave();

	switch (GET_MODE(fileattr)) {
	case IPMSG_FILE_REGULAR:
		trans.SendFileData(sock, iter);
		break;
	case IPMSG_FILE_DIR:
		trans.SendDirFiles(sock, iter);
		break;
	default:
		break;
	}
}

//传输 15,0 status,1 task,2 filename,3 side,4 finishsize,5 filesize,6 rate,7 progress,
//      8 terminate,9 packetn,10 fileid,11 size,12 type,13,dst,14 data
//terminate,0 Runing,1 Over,-1 Interrupt
GtkTreeModel *Transport::CreateTransModel()
{
	GtkListStore *model;

	model = gtk_list_store_new(15,
				   GDK_TYPE_PIXBUF, G_TYPE_STRING,
				   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
				   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT,
				   G_TYPE_INT, G_TYPE_UINT, G_TYPE_UINT,
				   G_TYPE_UINT64, G_TYPE_UINT, G_TYPE_STRING,
				   G_TYPE_POINTER);

	return GTK_TREE_MODEL(model);
}

bool Transport::CheckExist()
{
	if (!transport)
		return false;
	gtk_window_present(GTK_WINDOW(transport));
	return true;
}

void Transport::CreateTransView()
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	trans_view = gtk_tree_view_new_with_model(trans_model);
	g_signal_connect(trans_view, "button-press-event",
			 G_CALLBACK(PopupControlMenu), this);
	gtk_widget_show(trans_view);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("state"));
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "pixbuf", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(trans_view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("task"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(trans_view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("filename"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(trans_view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("pal"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 3, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(trans_view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("finished"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 4, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(trans_view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("length"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 5, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(trans_view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("rate"));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 6, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(trans_view), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_title(column, _("progress"));
	renderer = gtk_cell_renderer_progress_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, renderer, "value", 7, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(trans_view), column);
}

void Transport::CreateTransDialog()
{
	GtkWidget *box, *sw;
	GtkWidget *hbb, *button;

	transport = create_window(_("Transmission management"), 132, 79);
	gtk_container_set_border_width(GTK_CONTAINER(transport), 5);
	g_signal_connect(transport, "destroy", G_CALLBACK(DestroyDialog), NULL);

	box = create_box();
	gtk_container_add(GTK_CONTAINER(transport), box);

	sw = create_scrolled_window();
	gtk_box_pack_start(GTK_BOX(box), sw, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(sw), trans_view);

	hbb = create_button_box(FALSE);
	gtk_box_pack_start(GTK_BOX(box), hbb, FALSE, FALSE, 0);
	button = gtk_button_new_from_stock(GTK_STOCK_CLEAR);
	gtk_widget_show(button);
	g_signal_connect_swapped(button, "clicked", G_CALLBACK(TidyTask), this);
	gtk_box_pack_start(GTK_BOX(hbb), button, FALSE, FALSE, 0);
}

void Transport::RecvFileData(GtkTreeIter * iter)
{
	uint64_t filesize, finishsize;
	uint32_t packetno, fileid;
	gchar *filename, *pathname;
	char buf[MAX_SOCKBUF];
	my_file mf(true);
	Command cmd;
	int sock, fd;
	Pal *pal;

	gdk_threads_enter();
	gtk_tree_model_get(trans_model, iter, 2, &filename, 9, &packetno,
		   10, &fileid, 11, &filesize, 13, &pathname, 14, &pal, -1);
	gdk_threads_leave();

	sock = Socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	mf.chdir(pathname);
	fd = mf.open(filename, O_WRONLY | O_CREAT | O_TRUNC | O_LARGEFILE,
								     00644);
	g_free(filename), g_free(pathname);
	if (fd == -1 || !cmd.SendAskData(sock, pal, packetno, fileid, 0)) {
		EndTransportData(sock, fd, iter, __TIP_DIR "/error.png");
		return;
	}

	finishsize = RecvData(sock, fd, iter, filesize, buf, 0);
	if (finishsize >= filesize)
		EndTransportData(sock, fd, iter, __TIP_DIR "/finish.png");
	else
		EndTransportData(sock, fd, iter, __TIP_DIR "/error.png");
}

void Transport::RecvDirFiles(GtkTreeIter * iter)
{
	uint64_t filesize, finishsize, sumsize, dirsize;
	uint32_t packetno, fileid, headsize, fileattr;
	gchar *dirname, *pathname, *filename;
	char buf[MAX_SOCKBUF], *ptr;
	my_file mf(true);
	Command cmd;
	gint terminate;
	int sock, fd;
	int layers;
	ssize_t size;
	bool result;
	size_t len;
	Pal *pal;

	gdk_threads_enter();
	gtk_tree_model_get(trans_model, iter, 2, &dirname, 9, &packetno,
		   10, &fileid, 11, &dirsize, 13, &pathname, 14, &pal, -1);
	gdk_threads_leave();

	sock = Socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	mf.chdir(pathname), g_free(pathname);
	if (!cmd.SendAskFiles(sock, pal, packetno, fileid)) {
		EndTransportData(sock, -1, iter, __TIP_DIR "/error.png");
		g_free(dirname);
		return;
	}

	layers = 0, len = 0, sumsize = 0, result = false;
	while (1) {
		if ((size = read_ipmsg_fileinfo(sock, buf, MAX_SOCKBUF, len))
								   == -1)
			break;
		headsize = iptux_get_hex_number(buf, 0);
		pathname = ipmsg_get_filename(buf, 1);
		filesize = iptux_get_hex64_number(buf, 2);
		fileattr = iptux_get_hex_number(buf, 3);

		len = size - headsize;
		if (!FLAG_ISSET(pal->FlagsQuote(), 0)) {
			filename = transfer_encode(pathname,
					pal->EncodeQuote(), false);
			free(pathname);
		} else
			filename = pathname;
		if (GET_MODE(fileattr) != IPMSG_FILE_RETPARENT) {
			ptr = number_to_string_size(filesize);
			gdk_threads_enter();
			gtk_list_store_set(GTK_LIST_STORE(trans_model), iter, 2,
					   filename, 4, "0B", 5, ptr, 6, "0B/s",
					   7, 0, -1);
			gtk_tree_model_get(trans_model, iter, 8, &terminate, -1);
			gdk_threads_leave();
			free(ptr);
			if (terminate == -1)
				goto end;
		}

		if (GET_MODE(fileattr) == IPMSG_FILE_DIR) {
			mf.chdir((layers == 0) ? dirname : filename);
			layers++, free(filename);
			memmove(buf, buf + headsize, len);
			continue;
		} else if (GET_MODE(fileattr) == IPMSG_FILE_RETPARENT) {
			mf.chdir(".."), free(filename);
			memmove(buf, buf + headsize, len);
			if ((--layers) <= 0)
				break;
			continue;
		} else if (GET_MODE(fileattr) == IPMSG_FILE_REGULAR) {
			fd = mf.open(filename,
				     O_WRONLY | O_CREAT | O_TRUNC | O_LARGEFILE,
				     00644);
			free(filename);
			if (fd == -1)
				goto end;
		} else {
			free(filename);
			if ((fd = Open("/dev/null", O_WRONLY)) == -1)
				goto end;
		}
		size = len < filesize ? len : filesize;
		if (Write(fd, buf + headsize, size) == -1) {
			close(fd);
			goto end;
		}
		if (size == filesize) {
			sumsize += size, close(fd);
			if (len -= size)
				memmove(buf, buf + headsize + size, len);
			ptr = number_to_string_size(size);
			gdk_threads_enter();
			gtk_list_store_set(GTK_LIST_STORE(trans_model), iter,
					   4, ptr, 7, 100, -1);
			gdk_threads_leave();
			free(ptr);
			continue;
		}
		finishsize = RecvData(sock, fd, iter, filesize, buf, size);
		close(fd), len = 0;
		if (finishsize < filesize)
			goto end;
		sumsize += finishsize;
	}
	result = true;

 end:	if (result) {
		EndTransportData(sock, -1, iter, __TIP_DIR "/finish.png");
		EndTransportDirFiles(iter, dirname, sumsize, dirsize);
	} else {
		EndTransportData(sock, -1, iter, __TIP_DIR "/error.png");
		g_free(dirname);
	}
}

uint32_t Transport::RecvData(int sock, int fd, GtkTreeIter * iter,
			     uint64_t filesize, char *buf, uint32_t offset)
{
	uint64_t oldSize, downSize;
	struct timeval val1, val2;
	char *ptr1, *ptr2;
	float difftime;
	gint terminate;
	ssize_t size;

	if (offset == filesize) {
		gdk_threads_enter();
		gtk_list_store_set(GTK_LIST_STORE(trans_model), iter, 7, 100, -1);
		gdk_threads_leave();
		return filesize;
	}

	oldSize = downSize = offset;
	gettimeofday(&val1, NULL);
	do {
		size = (MAX_SOCKBUF < filesize - downSize) ?
		    MAX_SOCKBUF : filesize - downSize;
		if ((size = Read(sock, buf, size)) == -1)
			return downSize;
		if (size > 0 && Write(fd, buf, size) == -1)
			return downSize;
		downSize += size;
		gettimeofday(&val2, NULL);
		difftime = difftimeval(val2, val1);
		if (difftime >= 1) {
			ptr1 = number_to_string_size(downSize);
			ptr2 = number_to_string_size(
				   (uint64_t)((downSize - oldSize) / difftime),
				   true);
			gdk_threads_enter();
			gtk_list_store_set(GTK_LIST_STORE(trans_model),
					   iter, 4, ptr1, 6, ptr2, 7,
					   GINT(percent(downSize, filesize)), -1);
			gtk_tree_model_get(trans_model, iter, 8, &terminate, -1);
			gdk_threads_leave();
			g_free(ptr1), g_free(ptr2);
			if (terminate == -1)
				return downSize;
			val1 = val2;
			oldSize = downSize;
		}
	} while (size && downSize < filesize);

	ptr1 = number_to_string_size(downSize);
	gdk_threads_enter();
	gtk_list_store_set(GTK_LIST_STORE(trans_model), iter, 4, ptr1,
			   7, GINT(percent(downSize, filesize)), -1);
	gdk_threads_leave();
	free(ptr1);

	return downSize;
}

void Transport::SendFileData(int sock, GtkTreeIter * iter)
{
	gchar *filename, *pathname;
	char buf[MAX_SOCKBUF];
	uint64_t filesize, finishsize;
	my_file mf(false);
	int fd;

	gdk_threads_enter();
	gtk_tree_model_get(trans_model, iter, 2, &filename, 11, &filesize,
			   13, &pathname, -1);
	gdk_threads_leave();

	mf.chdir(pathname);
	fd = mf.open(filename, O_RDONLY | O_LARGEFILE);
	g_free(filename), g_free(pathname);
	if (fd == -1) {
		EndTransportData(-1, -1, iter, __TIP_DIR "/error.png");
		return;
	}

	finishsize = SendData(sock, fd, iter, filesize, buf);
	if (finishsize >= filesize)
		EndTransportData(-1, fd, iter, __TIP_DIR "/finish.png");
	else
		EndTransportData(-1, fd, iter, __TIP_DIR "/error.png");
}

void Transport::SendDirFiles(int sock, GtkTreeIter * iter)
{
	gchar *dirname, *pathname, *filename;
	char buf[MAX_SOCKBUF], *ptr;
	uint64_t finishsize, sumsize, dirsize;
	uint32_t headsize;
	struct dirent *dirt, vdirt;
	struct stat64 st;
	my_file mf(false);
	GQueue *dir_stack;
	gint terminate;
	bool result;
	Pal *pal;
	DIR *dir;
	int fd;

	gdk_threads_enter();
	gtk_tree_model_get(trans_model, iter, 2, &dirname, 11, &dirsize,
			   13, &pathname, 14, &pal, -1);
	gdk_threads_leave();
	mf.chdir(pathname), g_free(pathname);

	sumsize = 0, result = false, dir = NULL;
	strcpy(vdirt.d_name, dirname), dirt = &vdirt;
	dir_stack = g_queue_new();
	goto start;
	while (!g_queue_is_empty(dir_stack)) {
		dir = (DIR *) g_queue_pop_head(dir_stack);
		while (dir && (dirt = readdir(dir))) {
			if (strcmp(dirt->d_name, ".") == 0
			    || strcmp(dirt->d_name, "..") == 0)
				continue;

 start:		if (mf.stat(dirt->d_name, &st) == -1
			     || !S_ISREG(st.st_mode) && !S_ISDIR(st.st_mode))
				continue;
			ptr = number_to_string_size(st.st_size);
			gdk_threads_enter();
			gtk_list_store_set(GTK_LIST_STORE(trans_model), iter, 2,
					   dirt->d_name, 4, "0B", 5, ptr, 6,
					   "0B/s", 7, 0, -1);
			gtk_tree_model_get(trans_model, iter, 8, &terminate, -1);
			gdk_threads_leave();
			free(ptr);
			if (terminate == -1)
				goto end;

			pathname = transfer_encode(dirt->d_name,
					pal->EncodeQuote(), true);
			filename = ipmsg_set_filename_pal(pathname);
# if __WORDSIZE == 64
			snprintf(buf, MAX_SOCKBUF, "000:%s:%lx:%x:",
# else
			snprintf(buf, MAX_SOCKBUF, "000:%s:%llx:%x:",
# endif
				 filename, st.st_size,
				 S_ISREG(st.st_mode) ? IPMSG_FILE_REGULAR :
				 IPMSG_FILE_DIR);
			free(filename), free(pathname);
			headsize = strlen(buf);
			snprintf(buf, MAX_SOCKBUF, "%.3x", headsize);
			*(buf + strlen(buf)) = ':';
			if (Write(sock, buf, headsize) == -1)
				goto end;

			if (S_ISREG(st.st_mode)) {
				if ((fd = mf.open(dirt->d_name,
					  O_RDONLY | O_LARGEFILE)) == -1)
					goto end;
				finishsize = SendData(sock, fd, iter, st.st_size, buf);
				close(fd);
				if (finishsize < st.st_size)
					goto end;
				sumsize += finishsize;
			} else if (S_ISDIR(st.st_mode)) {
				mf.chdir(dirt->d_name);
				if (dir)
					g_queue_push_head(dir_stack, dir);
				if (!(dir = mf.opendir()))
					goto end;
			}
		}
		closedir(dir), dir = NULL;

		snprintf(buf, MAX_SOCKBUF, "000:.:0:%x:", IPMSG_FILE_RETPARENT);
		headsize = strlen(buf);
		snprintf(buf, MAX_SOCKBUF, "%.3x", headsize);
		*(buf + strlen(buf)) = ':';
		if (Write(sock, buf, headsize) == -1)
			goto end;
		mf.chdir("..");
	}
	result = true;

 end:	if (result) {
		EndTransportData(-1, -1, iter, __TIP_DIR "/finish.png");
		EndTransportDirFiles(iter, dirname, sumsize, dirsize);
	} else {
		closedir(dir);
		g_queue_foreach(dir_stack, GFunc(closedir), NULL);
		g_queue_clear(dir_stack);
		EndTransportData(-1, -1, iter, __TIP_DIR "/error.png");
		g_free(dirname);
	}
	g_queue_free(dir_stack);
}

uint32_t Transport::SendData(int sock, int fd, GtkTreeIter * iter,
			     uint64_t filesize, char *buf)
{
	uint64_t oldSize, sendSize;
	struct timeval val1, val2;
	char *ptr1, *ptr2;
	float difftime;
	gint terminate;
	ssize_t size;

	if (filesize == 0) {
		gdk_threads_enter();
		gtk_list_store_set(GTK_LIST_STORE(trans_model), iter, 7, 100, -1);
		gdk_threads_leave();
		return 0;
	}

	oldSize = sendSize = 0;
	gettimeofday(&val1, NULL);
	do {
		if ((size = Read(fd, buf, MAX_SOCKBUF)) == -1)
			return sendSize;
		if (size > 0 && Write(sock, buf, size) == -1)
			return sendSize;
		sendSize += size;
		gettimeofday(&val2, NULL);
		difftime = difftimeval(val2, val1);
		if (difftime >= 1) {
			ptr1 = number_to_string_size(sendSize);
			ptr2 = number_to_string_size(
				(uint64_t)((sendSize - oldSize) / difftime),
				 true);
			gdk_threads_enter();
			gtk_list_store_set(GTK_LIST_STORE(trans_model),
					   iter, 4, ptr1, 6, ptr2, 7,
					   GINT(percent(sendSize, filesize)), -1);
			gtk_tree_model_get(trans_model, iter, 8, &terminate, -1);
			gdk_threads_leave();
			g_free(ptr1), g_free(ptr2);
			if (terminate == -1)
				return sendSize;
			val1 = val2;
			oldSize = sendSize;
		}
	} while (size);

	ptr1 = number_to_string_size(sendSize);
	gdk_threads_enter();
	gtk_list_store_set(GTK_LIST_STORE(trans_model), iter, 4, ptr1,
			   7, GINT(percent(sendSize, filesize)), -1);
	gdk_threads_leave();
	free(ptr1);

	return sendSize;
}

void Transport::EndTransportData(int sock, int fd, GtkTreeIter * iter,
				 const char *pathname)
{
	GdkPixbuf *pixbuf;

	close(sock), close(fd);
	gdk_threads_enter();
	pixbuf = gdk_pixbuf_new_from_file(pathname, NULL);
	gtk_list_store_set(GTK_LIST_STORE(trans_model), iter,
					   0, pixbuf, 8, 1, -1);
	if (pixbuf)
		g_object_unref(pixbuf);
	gdk_threads_leave();
}

void Transport::EndTransportDirFiles(GtkTreeIter * iter, char *filename,
				     uint64_t finishsize, uint64_t filesize)
{
	char *ptr1, *ptr2;

	ptr1 = number_to_string_size(finishsize);
	ptr2 = number_to_string_size(filesize);
	gdk_threads_enter();
	gtk_list_store_set(GTK_LIST_STORE(trans_model), iter, 2, filename,
			   4, ptr1, 5, ptr2, -1);
	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(trans_view));
	gdk_threads_leave();
	free(filename), free(ptr1), free(ptr2);
}

GtkWidget *Transport::CreatePopupMenu()
{
	GtkWidget *menu, *menu_item;

	menu = gtk_menu_new();
	gtk_widget_show(menu);

	menu_item = gtk_menu_item_new_with_label(_("Terminate Job"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(StopTask), this);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Terminate All"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(StopAllTask), this);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label(_("Clear Tasklist"));
	g_signal_connect_swapped(menu_item, "activate",
				 G_CALLBACK(TidyTask), this);
	gtk_widget_show(menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	return menu;
}

void Transport::DestroyDialog()
{
	extern Transport trans;

	trans.transport = NULL;
	trans.trans_view = NULL;
}

gboolean Transport::PopupControlMenu(GtkWidget * view, GdkEventButton * event,
				     gpointer data)
{
	Transport *trans;
	GtkTreePath *path;

	if (event->button != 3)
		return FALSE;
	trans = (Transport *) data;
	if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(view), GINT(event->x),
				  GINT(event->y), &path, NULL, NULL, NULL)) {
		trans->flag = true;
		gtk_tree_model_get_iter(trans->trans_model, &trans->opt_iter, path);
		gtk_tree_path_free(path);
	} else
		trans->flag = false;
	gtk_menu_popup(GTK_MENU(trans->CreatePopupMenu()), NULL, NULL, NULL,
		       NULL, event->button, event->time);

	return TRUE;
}

void Transport::StopTask(gpointer data)
{
	Transport *trans;

	trans = (Transport *) data;
	if (!trans->flag)
		return;
	gtk_list_store_set(GTK_LIST_STORE(trans->trans_model),
				   &trans->opt_iter, 8, -1, -1);
}

void Transport::StopAllTask(gpointer data)
{
	GtkTreeIter iter;
	Transport *trans;

	trans = (Transport *) data;
	if (!gtk_tree_model_get_iter_first(trans->trans_model, &iter))
		return;
	do {
		gtk_list_store_set(GTK_LIST_STORE(trans->trans_model),
							   &iter, 8, -1, -1);
	} while (gtk_tree_model_iter_next(trans->trans_model, &iter));
}

void Transport::TidyTask(gpointer data)
{
	gboolean result;
	gint terminate;
	GtkTreeIter iter;
	Transport *trans;

	trans = (Transport *) data;
	if (!gtk_tree_model_get_iter_first(trans->trans_model, &iter))
		return;
	do {
mark:		gtk_tree_model_get(trans->trans_model, &iter, 8, &terminate, -1);
		if (terminate == 1) {
			result = gtk_list_store_remove(
				    GTK_LIST_STORE(trans->trans_model), &iter);
			if (result)
				goto mark;
			break;
		}
	} while (gtk_tree_model_iter_next(trans->trans_model, &iter));
	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(trans->trans_view));
}
