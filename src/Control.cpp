//
// C++ Implementation: Control
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Control.h"
#include "my_file.h"
#include "utils.h"
#include "baling.h"

 Control::Control():netseg(NULL), palicon(NULL), myicon(NULL),
myname(NULL), mygroup(NULL), encode(NULL), path(NULL),
font(NULL), sign(NULL), flags(0), dirty(false), table(NULL),
iconlist(NULL), pix(3.4)
{
	pthread_mutex_init(&mutex, NULL);
}

Control::~Control()
{
	pthread_mutex_lock(&mutex);
	g_slist_foreach(netseg, GFunc(remove_foreach),
			GINT_TO_POINTER(NETSEGMENT));
	g_slist_free(netseg);
	g_slist_foreach(iconlist, GFunc(remove_foreach),
			GINT_TO_POINTER(SYSICON));
	g_slist_free(iconlist);
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);

	free(palicon);
	free(myicon);
	free(myname);
	free(mygroup);
	free(encode);
	free(path);
	free(font);
	free(sign);

	g_object_unref(table);
}

void Control::InitSelf()
{
	ReadControl();
	CreateTagTable();
	GetSysIcon();
	GetRatio_PixMm();
}

void Control::WriteControl()
{
	GConfClient *client;

	client = gconf_client_get_default();
	UpdateNetSegment(client, true);
	gconf_client_set_string(client, GCONF_PATH "/pal_icon", palicon, NULL);
	gconf_client_set_string(client, GCONF_PATH "/self_icon", myicon, NULL);
	gconf_client_set_string(client, GCONF_PATH "/nick_name", myname, NULL);
	gconf_client_set_string(client, GCONF_PATH "/belong_group", mygroup,
				NULL);
	gconf_client_set_string(client, GCONF_PATH "/net_encode", encode, NULL);
	gconf_client_set_string(client, GCONF_PATH "/save_path", path, NULL);
	gconf_client_set_string(client, GCONF_PATH "/panel_font", font, NULL);
	gconf_client_set_string(client, GCONF_PATH "/personal_sign", sign,
				NULL);
	gconf_client_set_bool(client, GCONF_PATH "/not_sound_support",
			      FLAG_ISSET(flags, 6) ? TRUE : FALSE, NULL);
	gconf_client_set_bool(client, GCONF_PATH "/min_memory_usage",
			      FLAG_ISSET(flags, 5) ? TRUE : FALSE, NULL);
	gconf_client_set_bool(client, GCONF_PATH "/use_enter_key",
			      FLAG_ISSET(flags, 4) ? TRUE : FALSE, NULL);
	gconf_client_set_bool(client, GCONF_PATH "/clearup_history",
			      FLAG_ISSET(flags, 3) ? TRUE : FALSE, NULL);
	gconf_client_set_bool(client, GCONF_PATH "/record_log",
			      FLAG_ISSET(flags, 2) ? TRUE : FALSE, NULL);
	gconf_client_set_bool(client, GCONF_PATH "/open_blacklist",
			      FLAG_ISSET(flags, 1) ? TRUE : FALSE, NULL);
	gconf_client_set_bool(client, GCONF_PATH "/proof_shared",
			      FLAG_ISSET(flags, 0) ? TRUE : FALSE, NULL);
	g_object_unref(client);

	dirty = false;
}

void Control::AdjustMemory()
{
	GSList *tmp;
	SysIcon *si;

	tmp = iconlist;
	while (tmp) {
		si = (SysIcon *) tmp->data;
		gdk_threads_enter();
		if (FLAG_ISSET(flags, 5)) {	//最小化内存使用
			if (si->pixbuf) {
				g_object_unref(si->pixbuf);
				si->pixbuf = NULL;
			}
		} else {	//以内存换取运行效率
			if (!si->pixbuf)
				si->pixbuf = gdk_pixbuf_new_from_file_at_size(
						  si->pathname, MAX_ICONSIZE,
						  MAX_ICONSIZE, NULL);
		}
		gdk_threads_leave();
		tmp = tmp->next;
	}
}

GSList *Control::CopyNetSegment()
{
	NetSegment *ns, *pns;
	GSList *list, *tmp;

	pthread_mutex_lock(&mutex);
	list = NULL, tmp = netseg;
	while (tmp) {
		pns = (NetSegment *) tmp->data;
		ns = new NetSegment(Strdup(pns->startip),
				    Strdup(pns->endip), NULL);
		list = g_slist_append(list, ns);
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&mutex);

	return list;
}

char *Control::FindNetSegDescribe(in_addr_t ipv4)
{
	in_addr_t ip1, ip2;
	NetSegment *ns;
	GSList *tmp;
	char *describe;

	ipv4 = ntohl(ipv4);
	pthread_mutex_lock(&mutex);
	tmp = netseg, describe = NULL;
	while (tmp) {
		ns = (NetSegment *) tmp->data;
		inet_pton(AF_INET, ns->startip, &ip1);
		inet_pton(AF_INET, ns->endip, &ip2);
		ip1 = ntohl(ip1), ip2 = ntohl(ip2);
		ipv4_order(ip1, ip2);
		if (ipv4 >= ip1 && ipv4 <= ip2) {
			describe = Strdup(ns->describe);
			break;
		}
		tmp = tmp->next;
	}
	pthread_mutex_unlock(&mutex);

	return describe ? describe : Strdup("");
}

void Control::ReadControl()
{
	GConfClient *client;

	client = gconf_client_get_default();
	UpdateNetSegment(client, false);
	if (!(palicon =
	      gconf_client_get_string(client, GCONF_PATH "/pal_icon", NULL)))
		palicon = Strdup(__ICON_DIR "/qq.png");
	if (!(myicon =
	      gconf_client_get_string(client, GCONF_PATH "/self_icon", NULL)))
		myicon = Strdup(__ICON_DIR "/tux.png");
	if (!(myname =
	      gconf_client_get_string(client, GCONF_PATH "/nick_name", NULL)))
		myname = Strdup(g_get_user_name());
	if (!(mygroup =
	      gconf_client_get_string(client, GCONF_PATH "/belong_group", NULL)))
		mygroup = Strdup("");
	if (!(encode =
	      gconf_client_get_string(client, GCONF_PATH "/net_encode", NULL)))
		encode = Strdup(_("UTF-8"));
	if (!(path =
	      gconf_client_get_string(client, GCONF_PATH "/save_path", NULL)))
		path = Strdup(g_get_home_dir());
	if (!(font =
	      gconf_client_get_string(client, GCONF_PATH "/panel_font", NULL)))
		font = Strdup("Sans Italic 10");
	if (!(sign =
	      gconf_client_get_string(client, GCONF_PATH "/personal_sign", NULL)))
		sign = Strdup("");
	if (gconf_client_get_bool(client, GCONF_PATH "/not_sound_support", NULL))
		FLAG_SET(flags, 6);
	if (gconf_client_get_bool(client, GCONF_PATH "/min_memory_usage", NULL))
		FLAG_SET(flags, 5);
	if (gconf_client_get_bool(client, GCONF_PATH "/use_enter_key", NULL))
		FLAG_SET(flags, 4);
	if (gconf_client_get_bool(client, GCONF_PATH "/clearup_history", NULL))
		FLAG_SET(flags, 3);
	if (gconf_client_get_bool(client, GCONF_PATH "/record_log", NULL))
		FLAG_SET(flags, 2);
	if (gconf_client_get_bool(client, GCONF_PATH "/open_blacklist", NULL))
		FLAG_SET(flags, 1);
	if (gconf_client_get_bool(client, GCONF_PATH "/proof_shared", NULL))
		FLAG_SET(flags, 0);
	g_object_unref(client);

	dirty = true;
}

void Control::CreateTagTable()
{
	GtkTextTag *tag;

	table = gtk_text_tag_table_new();
	tag = gtk_text_tag_new("blue");
	g_object_set(tag, "foreground", "blue", NULL);
	gtk_text_tag_table_add(table, tag);
	tag = gtk_text_tag_new("green");
	g_object_set(tag, "foreground", "green", NULL);
	gtk_text_tag_table_add(table, tag);
	tag = gtk_text_tag_new("red");
	g_object_set(tag, "foreground", "red", NULL);
	gtk_text_tag_table_add(table, tag);
	tag = gtk_text_tag_new("sign");
	g_object_set(tag, "indent", 10, "foreground", "#1005F0",
				     "font", "Sans Italic 8", NULL);
	gtk_text_tag_table_add(table, tag);
}

void Control::GetSysIcon()
{
	my_file mf(false);
	struct dirent *dirt;
	char path[MAX_PATHBUF];
	DIR *dir;

	mf.chdir(__ICON_DIR);
	if (!(dir = mf.opendir()))
		return;
	while (dirt = readdir(dir)) {
		if (strcmp(dirt->d_name, ".") == 0
		    || strcmp(dirt->d_name, "..") == 0)
			continue;
		snprintf(path, MAX_PATHBUF, __ICON_DIR "/%s", dirt->d_name);
		iconlist = g_slist_append(iconlist,
			  new SysIcon(Strdup(path), NULL));		//延迟到守护线程完成所有工作
	}
	closedir(dir);

	/* 当使用好友自定义头像时，进一步节俭内存 */
	if (strncmp(palicon, __ICON_DIR, strlen(__ICON_DIR)) != 0)
		iconlist = g_slist_prepend(iconlist,
			   new SysIcon(Strdup(palicon), NULL));
}

void Control::GetRatio_PixMm()
{
	GdkScreen *screen;
	gint width, widthmm;

	screen = gdk_screen_get_default();
	width = gdk_screen_get_width(screen);
	widthmm = gdk_screen_get_width_mm(screen);

	pix = (float)width / widthmm;
}

//写出 TRUE,读入 FALSE
void Control::UpdateNetSegment(GConfClient * client, bool direc)
{
	NetSegment *ns;
	GSList *list, *tmp;

	pthread_mutex_lock(&mutex);
	if (direc) {
		list = NULL, tmp = netseg;
		while (tmp) {
			ns = (NetSegment *) tmp->data;
			list = g_slist_append(list, ns->startip);
			list = g_slist_append(list, ns->endip);
			list = g_slist_append(list, ns->describe);
			tmp = tmp->next;
		}
		gconf_client_set_list(client, GCONF_PATH "/scan_net_segment",
				      GCONF_VALUE_STRING, list, NULL);
	} else {
		tmp = list =
		    gconf_client_get_list(client, GCONF_PATH "/scan_net_segment",
						  GCONF_VALUE_STRING, NULL);
		while (tmp) {
			ns = new NetSegment(NULL, NULL, NULL);
			ns->startip = (char *)tmp->data;
			tmp = tmp->next;
			ns->endip = (char *)tmp->data;
			tmp = tmp->next;
			ns->describe = tmp->data ? (char *)tmp->data : Strdup("");
			tmp = tmp->next;
			netseg = g_slist_append(netseg, ns);
		}
	}
	pthread_mutex_unlock(&mutex);
	g_slist_free(list);
}
