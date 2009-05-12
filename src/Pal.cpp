//
// C++ Implementation: Pal
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Pal.h"
#include "Control.h"
#include "Command.h"
#include "UdpData.h"
#include "RecvFile.h"
#include "DialogPeer.h"
#include "Log.h"
#include "support.h"
#include "baling.h"
#include "utils.h"

 Pal::Pal():ipv4(0), segment(NULL), version(NULL), packetn(0),
user(NULL), host(NULL), name(NULL), group(NULL), ad(NULL),
sign(NULL), iconfile(NULL), encode(NULL), flags(0), tpointer(NULL),
iconpix(NULL), dialog(NULL), mypacketn(0), reply(true)
{
	extern Control ctr;
	record = gtk_text_buffer_new(ctr.table);
    urlregex = g_regex_new ("(http|ftp|https):\\/\\/[\\w\\-_]+(\\.[\\w\\-_]+)+([\\w\\-\\.,@?^=%&amp;:/~\\+#]*[\\w\\-\\@?^=%&amp;/~\\+#])?", (GRegexCompileFlags)0, (GRegexMatchFlags)0, NULL);
}

Pal::~Pal()
{
	SendExit();

	free(segment);
	free(version);
	free(user);
	free(host);
	free(name);
	free(group);
	free(ad);
	free(sign);
	free(iconfile);
	free(encode);
	free(tpointer);

	if (iconpix)
		g_object_unref(iconpix);
	if (dialog)
		gtk_widget_destroy(dialog->DialogQuote());
	g_object_unref(record);

    g_regex_unref (urlregex);
}

//entry 是否为通知登录消息,true 必须转换编码;false 情况而定
void Pal::CreateInfo(in_addr_t ip, const char *msg, size_t size, bool entry)
{
	extern Control ctr;
	char *ptr;

	ipv4 = ip;
	segment = ctr.FindNetSegDescribe(ipv4);
	if (!IptuxGetIcon(msg, size))
		iconfile = Strdup(ctr.palicon);
	if (!IptuxGetEncode(msg, size))
		encode = Strdup(ctr.encode);
	if (!IptuxGetGroup(msg, size, entry))	//依赖编码支持
		group = Strdup("");

	if (!entry) {
		if (FLAG_ISSET(flags, 0))
			ptr = Strdup(msg);
		else		//当用户修正过此编码后发挥效用
			ptr = transfer_encode(msg, encode, false);
	} else
		ptr = transfer_encode(msg, ctr.encode, false);
	version = iptux_get_section_string(ptr, 0);
	packetn = 0;
	user = iptux_get_section_string(ptr, 2);
	host = iptux_get_section_string(ptr, 3);
	name = ipmsg_get_attach(ptr, 5);
	free(ptr);
	FLAG_SET(flags, 1);
}

void Pal::UpdateInfo(const char *msg, size_t size, bool entry)
{
	extern Control ctr;
	char *ptr;
	bool cpt;

	free(segment);
	segment = ctr.FindNetSegDescribe(ipv4);

	if (entry || !FLAG_ISSET(flags, 2)) {
		cpt = FLAG_ISSET(flags, 0);
		ptr = iconfile;
		if (IptuxGetIcon(msg, size))
			free(ptr);
		ptr = encode;
		if (IptuxGetEncode(msg, size))
			free(ptr);
		if (entry && !FLAG_ISSET(flags, 0)) {
			free(encode);
			encode = Strdup(ctr.encode);
		} else if (cpt)		//如果以前兼容，则此次也兼容
			FLAG_SET(flags, 0);
		ptr = group;
		if (IptuxGetGroup(msg, size, entry))	//依赖兼容性支持
			free(ptr);
	}

	if (!entry) {
		if (FLAG_ISSET(flags, 0))
			ptr = Strdup(msg);
		else
			ptr = transfer_encode(msg, encode, false);
	} else
		ptr = transfer_encode(msg, ctr.encode, false);
	free(version);
	version = iptux_get_section_string(ptr, 0);
	packetn = 0;
	free(user);
	user = iptux_get_section_string(ptr, 2);
	free(host);
	host = iptux_get_section_string(ptr, 3);
	if (entry || !FLAG_ISSET(flags, 2)) {
		free(name);
		name = ipmsg_get_attach(ptr, 5);
	}
	free(ptr);
	FLAG_SET(flags, 1);
}

GdkPixbuf *Pal::GetIconPixbuf()
{
	extern Control ctr;
	GSList *tmp;
	SysIcon *si;

	if (iconpix) {	//对象存在
		if (tpointer && strcmp(tpointer, iconfile) == 0) {	//最新
			g_object_ref(iconpix);
			return iconpix;
		} else {	//否则
			g_object_unref(iconpix);
			iconpix = NULL;
		}
	}

	free(tpointer);
	tpointer = Strdup(iconfile);

	tmp = ctr.iconlist;
	while (tmp) {	//查询是否为系统图标
		si = (SysIcon *) tmp->data;
		if (strcmp(si->pathname, iconfile) == 0)
			break;
		tmp = tmp->next;
	}
	if (!tmp) {	//不是系统图标
		iconpix = gdk_pixbuf_new_from_file_at_size(iconfile,
				MAX_ICONSIZE, MAX_ICONSIZE, NULL);
		if (iconpix)
			g_object_ref(iconpix);
		return iconpix;
	}

	//是系统图标，且已经加载进入内存
	if (si->pixbuf) {
		g_object_ref(si->pixbuf);
		iconpix = si->pixbuf;
		g_object_ref(si->pixbuf);
		return iconpix;
	}
	//是系统图标，但是未加载进入内存
	iconpix = gdk_pixbuf_new_from_file_at_size(iconfile,
			MAX_ICONSIZE, MAX_ICONSIZE, NULL);
	if (iconpix) {
		g_object_ref(iconpix);
		si->pixbuf = iconpix;
		g_object_ref(iconpix);
	}
	return iconpix;
}

bool Pal::CheckReply(uint32_t packetno, bool install)
{
	if (install) {
		mypacketn = packetno;
		reply = false;
		return true;
	}

	if (reply || mypacketn > packetno)
		return true;
	return false;
}

void Pal::BufferInsertData(GSList * chiplist, enum BELONG_TYPE type)
{
	switch (type) {
	case PAL:
		gdk_threads_enter();
		BufferInsertPal(chiplist);
		gdk_threads_leave();
		break;
	case SELF:
		BufferInsertSelf(chiplist);
		break;
	case ERROR:
		gdk_threads_enter();
		BufferInsertError(chiplist);
		gdk_threads_leave();
		break;
	default:
		break;
	}
}

void Pal::ViewScroll()
{
	GtkTextIter start, end;
	GtkTextMark *mark;

	if (!dialog)
		return;

	gtk_text_buffer_get_bounds(record, &start, &end);
	if (gtk_text_iter_equal(&start, &end))
		return;
	mark = gtk_text_buffer_create_mark(record, NULL, &end, FALSE);
	gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(dialog->ScrollQuote()),
				     mark, 0.0, TRUE, 0.0, 0.0);
	gtk_text_buffer_delete_mark(record, mark);
	gtk_window_present(GTK_WINDOW(dialog->DialogQuote()));
}

//是否已经将消息存放在缓冲区中
bool Pal::RecvMessage(const char *msg)
{
	extern UdpData udt;
	uint32_t packetno;
	GSList *chiplist;
	GList *tmp;
	char *ptr;

	packetno = iptux_get_dec_number(msg, 1);
	if (packetno <= packetn)
		return false;
	packetn = packetno;

	ptr = ipmsg_get_attach(msg, 5);
	if (!ptr || *ptr == '\0') {
		free(ptr);
		return true;
	}

	chiplist = g_slist_append(NULL, new ChipData(STRING, ptr));
	BufferInsertData(chiplist, PAL);
	g_slist_foreach(chiplist, GFunc(remove_foreach),
				GINT_TO_POINTER(CHIPDATA));
	g_slist_free(chiplist);

	if (!dialog) {
		tmp = (GList *) udt.PalGetMsgPos(this);
		if (!tmp) {
			pthread_mutex_lock(udt.MutexQuote());
			g_queue_push_tail(udt.MsgqueueQuote(), this);
			pthread_mutex_unlock(udt.MutexQuote());
		}
	}
	return true;
}

bool Pal::RecvAskShared(const char *msg)
{
	uint32_t packetno;

	packetno = iptux_get_dec_number(msg, 1);
	if (packetno <= packetn)
		return false;
	packetn = packetno;
	return true;
}

bool Pal::RecvIcon(const char *msg, size_t size)
{
	char file[MAX_PATHBUF];
	size_t len;
	int fd;

	if (FLAG_ISSET(flags, 2) || (len = strlen(msg) + 1) >= size)
		return false;
	snprintf(file, MAX_PATHBUF, "%s" ICON_PATH "/%" PRIx32,
				 g_get_user_cache_dir(), ipv4);
	if ((fd = Open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
		return false;
	Write(fd, msg + len, size - len);
	close(fd);
	free(iconfile);
	iconfile = Strdup(file);
	return true;
}

void Pal::RecvReply(const char *msg)
{
	uint32_t oldpacket;

	oldpacket = iptux_get_dec_number(msg, 5);
	if (oldpacket == mypacketn)
		reply = true;
}

void Pal::RecvFile(const char *msg, size_t size)
{
	extern Log mylog;
	struct recvfile_para *para;
	uint32_t commandno;
	const char *ptr;

	commandno = iptux_get_dec_number(msg, 4);
	if ((!(ptr = iptux_skip_string(msg, size, 1)) || *ptr == '\0')
		      && !(commandno & IPTUX_SHAREDOPT))
		return;
	para = (struct recvfile_para *)Malloc(sizeof(struct recvfile_para));
	para->data = this;
	para->msg = ptr ? transfer_encode(ptr, encode, false) : NULL;
	para->commandn = commandno;
	para->packetn = iptux_get_dec_number(msg, 1);
	thread_create(ThreadFunc(RecvFile::RecvEntry), para, false);
	mylog.SystemLog(_("Received a number of document information!"));
}

void Pal::RecvSign(const char *msg)
{
	char *ptr;

	ptr = ipmsg_get_attach(msg, 5);
	if (!FLAG_ISSET(flags, 0)) {
		sign = transfer_encode(ptr, encode, false);
		free(ptr);
	} else
		sign = ptr;
}

void Pal::RecvAdPic(const char *path)
{
	ad = Strdup(path);
}

void Pal::RecvMsgPic(const char *path)
{
	GSList *chiplist;

	chiplist = g_slist_append(NULL, new ChipData(PICTURE, Strdup(path)));
	BufferInsertData(chiplist, PAL);
	g_slist_foreach(chiplist, GFunc(remove_foreach),
				GINT_TO_POINTER(CHIPDATA));
	g_slist_free(chiplist);
}

void Pal::SendAnsentry()
{
	extern struct interactive inter;
	Command cmd;

	cmd.SendAnsentry(inter.udpsock, this);
}

void Pal::SendReply(const char *msg)
{
	extern struct interactive inter;
	Command cmd;
	uint32_t packetno;

	packetno = iptux_get_dec_number(msg, 1);
	cmd.SendReply(inter.udpsock, this, packetno);
}

void Pal::SendExit()
{
	extern struct interactive inter;
	Command cmd;

	cmd.SendExit(inter.udpsock, this);
}

//是否成功获得组名
bool Pal::IptuxGetGroup(const char *msg, size_t size, bool entry)
{
	extern Control ctr;
	const char *ptr;

	if (!(ptr = iptux_skip_string(msg, size, 1)) || *ptr == '\0')
		return false;

	//必须对编码作一定的处理
	if (!entry) {
		if (FLAG_ISSET(flags, 0))
			group = Strdup(ptr);
		else
			group = transfer_encode(ptr, encode, false);
	} else
		group = transfer_encode(ptr, ctr.encode, false);

	return true;
}

//是否成功获得头像文件名
bool Pal::IptuxGetIcon(const char *msg, size_t size)
{
	const char *ptr;
	char path[MAX_PATHBUF];

	if (!(ptr = iptux_skip_string(msg, size, 2)) || *ptr == '\0')
		return false;
	snprintf(path, MAX_PATHBUF, __ICON_PATH "/%s", ptr);
	if (access(path, F_OK) != 0)
		return false;
	iconfile = Strdup(path);
	return true;
}

//是否成功获得系统编码
bool Pal::IptuxGetEncode(const char *msg, size_t size)
{
	const char *ptr;

	FLAG_CLR(flags, 0);
	if (!(ptr = iptux_skip_string(msg, size, 3)) || *ptr == '\0')
		return false;
	FLAG_SET(flags, 0);
	encode = Strdup(ptr);
	return true;
}

void Pal::BufferInsertString(gchar *message)
{
    GMatchInfo *match_info;
    GtkTextIter viewend;
    GError *error = NULL;
    gint start_pos;
    gint end_pos;
    gint urlend = 0;

    g_regex_match_full (urlregex, message, -1, 0, (GRegexMatchFlags)0, &match_info, &error);
    while (g_match_info_matches (match_info))
    {
        gchar *word = g_match_info_fetch (match_info, 0);
        g_print ("Found: %s\n", word);

        GtkTextTag *tag = gtk_text_buffer_create_tag (record, NULL, 
                                    "foreground", "blue", 
                                    "underline", PANGO_UNDERLINE_SINGLE, 
                                    NULL);
        g_object_set_data (G_OBJECT (tag), "url",  word);
        g_match_info_fetch_pos(match_info, 0, &start_pos, &end_pos);
        
        //插入url前的字段
        gtk_text_buffer_get_end_iter(record, &viewend);
        gtk_text_buffer_insert(record, &viewend, message + urlend, start_pos - urlend);

        //插入url
        gtk_text_buffer_get_end_iter(record, &viewend);
        gtk_text_buffer_insert_with_tags (record, &viewend, message + start_pos, end_pos - start_pos, tag, NULL);

        urlend = end_pos;
//        g_print ("beg: %d, end: %d\n", start_pos, end_pos);
        g_match_info_next (match_info, &error);
    }
    g_match_info_free (match_info);
    if (error != NULL)
    {
        g_printerr ("Error while matching: %s\n", error->message);
        g_error_free (error);
    }

    //插入剩余字段
    gtk_text_buffer_get_end_iter(record, &viewend);
    gtk_text_buffer_insert(record, &viewend, message + urlend, -1);
    gtk_text_buffer_insert(record, &viewend, "\n", -1);
}


void Pal::BufferInsertPal(GSList * chiplist)
{
	extern Log mylog;
	GtkTextIter start, end;
	GdkPixbuf *pixbuf;
	char *ptr, *pptr, *header;
	GSList *tmp;

	tmp = chiplist;
	while (tmp) {
		ptr = ((ChipData *) tmp->data)->data;
		switch (((ChipData *) tmp->data)->type) {
		case STRING:
			header = getformattime("%s", name);
			gtk_text_buffer_get_end_iter(record, &end);
			gtk_text_buffer_insert_with_tags_by_name(record, &end,
						    header, -1, "blue", NULL);
			g_free(header);
			if (!FLAG_ISSET(flags, 0))
				pptr = transfer_encode(ptr, encode, false);
			else
				pptr = Strdup(ptr);
            // ========================================
		//	gtk_text_buffer_insert(record, &end, pptr, -1);
		//	gtk_text_buffer_insert(record, &end, "\n", -1);
            BufferInsertString(pptr);
			mylog.CommunicateLog(this, pptr);
            // ========================================
			free(pptr);
			break;
		case PICTURE:
			gtk_text_buffer_get_start_iter(record, &start);
			if (gtk_text_iter_get_char(&start) == OCCUPY_OBJECT
				|| gtk_text_iter_forward_find_char(&start,
					GtkTextCharPredicate(compare_foreach),
					GUINT_TO_POINTER(OCCUPY_OBJECT),
					NULL)) {
				end = start;
				gtk_text_iter_forward_char(&end);
				gtk_text_buffer_delete(record, &start, &end);
			} else {
				header = getformattime("%s", name);
				gtk_text_buffer_insert_with_tags_by_name(
				    record, &start, header, -1, "blue", NULL);
				gtk_text_buffer_insert(record, &start, "\n", -1);
				gtk_text_iter_backward_char(&start);
				g_free(header);
			}
			pixbuf = gdk_pixbuf_new_from_file(ptr, NULL);
			if (pixbuf) {
				gtk_text_buffer_insert_pixbuf(record,
							  &start, pixbuf);
				g_object_unref(pixbuf);
			}
			break;
		default:
			break;
		}
		tmp = tmp->next;
	}

	ViewScroll();
}

void Pal::BufferInsertSelf(GSList * chiplist)
{
	extern Control ctr;
	extern Log mylog;
	GtkTextIter start, end;
	GdkPixbuf *pixbuf;
	GSList *tmp;
	char *ptr;

	ptr = getformattime("%s", ctr.myname);
	gtk_text_buffer_get_end_iter(record, &end);
	gtk_text_buffer_insert_with_tags_by_name(record, &end,
					   ptr, -1, "green", NULL);
	g_free(ptr);

	tmp = chiplist;
	while (tmp) {
		ptr = ((ChipData *) tmp->data)->data;
		switch (((ChipData *) tmp->data)->type) {
		case STRING:
            // ===============================
		//	gtk_text_buffer_get_end_iter(record, &end);
		//	gtk_text_buffer_insert(record, &end, ptr, -1);
            BufferInsertString(ptr);
			mylog.CommunicateLog(NULL, ptr);
            // ===============================
			break;
		case PICTURE:
			gtk_text_buffer_get_start_iter(record, &start);
			if (gtk_text_iter_get_char(&start) == OCCUPY_OBJECT
				|| gtk_text_iter_forward_find_char(&start,
					GtkTextCharPredicate(compare_foreach),
					GUINT_TO_POINTER(OCCUPY_OBJECT),
					NULL)) {
				end = start;
				gtk_text_iter_forward_char(&end);
				gtk_text_buffer_delete(record, &start, &end);
			}
			pixbuf = gdk_pixbuf_new_from_file(ptr, NULL);
			if (pixbuf) {
				gtk_text_buffer_insert_pixbuf(record,
							&start, pixbuf);
				g_object_unref(pixbuf);
			}
			break;
		default:
			break;
		}
		tmp = tmp->next;
	}

	gtk_text_buffer_get_end_iter(record, &end);
	gtk_text_buffer_insert(record, &end, "\n", -1);
	ViewScroll();
}

//暂且不支持图片
void Pal::BufferInsertError(GSList * chiplist)
{
	extern Log mylog;
	GtkTextIter iter;
	GSList *tmp;
	gchar *ptr;

	gtk_text_buffer_get_end_iter(record, &iter);
	ptr = getformattime(_("<tips>"));
	gtk_text_buffer_insert_with_tags_by_name(record, &iter,
						ptr, -1, "red", NULL);
	g_free(ptr);

	tmp = chiplist;
	while (tmp) {
		if (((ChipData *) tmp->data)->type == STRING)
			gtk_text_buffer_insert_with_tags_by_name(record, &iter,
			      ((ChipData *) tmp->data)->data, -1, "red", NULL);
		tmp = tmp->next;
	}
	gtk_text_buffer_insert(record, &iter, "\n", -1);

	mylog.SystemLog(_("Send a message failed!"));
	ViewScroll();
}

void Pal::SendFeature(gpointer data)
{
	extern struct interactive inter;
	extern Control ctr;
	char path[MAX_PATHBUF];
	Command cmd;
	int sock;

	if (strncmp(ctr.myicon, __ICON_PATH, strlen(__ICON_PATH)))
		cmd.SendMyIcon(inter.udpsock, data);
	if (ctr.sign && *ctr.sign != '\0')
		cmd.SendMySign(inter.udpsock, data);
	snprintf(path, MAX_PATHBUF, "%s" COMPLEX_PATH "/ad",
					 g_get_user_config_dir());
	if (access(path, F_OK) == 0) {
		sock = Socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		cmd.SendSublayer(sock, data, IPTUX_ADPICOPT, path);
		close(sock);
	}
}
