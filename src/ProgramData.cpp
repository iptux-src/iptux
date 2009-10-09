//
// C++ Implementation: ProgramData
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "ProgramData.h"
#include "utils.h"

/**
 * 类构造函数.
 */
ProgramData::ProgramData():nickname(NULL), mygroup(NULL),
 myicon(NULL), path(NULL),  sign(NULL), codeset(NULL), encode(NULL),
 palicon(NULL), font(NULL), flags(0), transtip(NULL), msgtip(NULL),
 volume(1.0), sndfgs(~0), netseg(NULL), urlregex(NULL), xcursor(NULL),
 lcursor(NULL), table(NULL)
{
	pthread_mutex_init(&mutex, NULL);
}

/**
 * 类析构函数.
 */
ProgramData::~ProgramData()
{
	g_free(nickname);
	g_free(mygroup);
	g_free(myicon);
	g_free(path);
	g_free(sign);

	g_free(codeset);
	g_free(encode);
	g_free(palicon);
	g_free(font);

	g_free(msgtip);
	g_free(transtip);

	g_slist_foreach(netseg, GFunc(glist_delete_foreach),
				 GINT_TO_POINTER(NET_SEGMENT));
	g_slist_free(netseg);

	g_regex_unref(urlregex);
	gdk_cursor_unref(xcursor);
	gdk_cursor_unref(lcursor);
	g_object_unref(table);

	pthread_mutex_destroy(&mutex);
}

/**
 * 初始化相关类成员数据.
 */
void ProgramData::InitSublayer()
{
	ReadProgData();
	CheckIconTheme();
	CreateRegex();
	CreateCursor();
	CreateTagTable();
}

/**
 * 写出程序数据.
 */
void ProgramData::WriteProgData()
{
	GConfClient *client;

	client = gconf_client_get_default();

	gconf_client_set_string(client, GCONF_PATH "/nick_name", nickname, NULL);
	gconf_client_set_string(client, GCONF_PATH "/belong_group", mygroup, NULL);
	gconf_client_set_string(client, GCONF_PATH "/my_icon", myicon, NULL);
	gconf_client_set_string(client, GCONF_PATH "/archive_path", path, NULL);
	gconf_client_set_string(client, GCONF_PATH "/personal_sign", sign, NULL);

	gconf_client_set_string(client, GCONF_PATH "/candidacy_encode", codeset, NULL);
	gconf_client_set_string(client, GCONF_PATH "/preference_encode", encode, NULL);
	gconf_client_set_string(client, GCONF_PATH "/pal_icon", palicon, NULL);
	gconf_client_set_string(client, GCONF_PATH "/panel_font", font, NULL);
	gconf_client_set_bool(client, GCONF_PATH "/hide_startup",
			 FLAG_ISSET(flags, 6) ? TRUE : FALSE, NULL);
	gconf_client_set_bool(client, GCONF_PATH "/open_transmission",
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

	gconf_client_set_string(client, GCONF_PATH "/trans_tip", transtip, NULL);
	gconf_client_set_string(client, GCONF_PATH "/msg_tip", msgtip, NULL);
	gconf_client_set_float(client, GCONF_PATH "/volume_degree", volume, NULL);
	gconf_client_set_bool(client, GCONF_PATH "/msgsnd_support",
			 FLAG_ISSET(sndfgs, 2) ? TRUE : FALSE, NULL);
	gconf_client_set_bool(client, GCONF_PATH "/transnd_support",
			 FLAG_ISSET(sndfgs, 1) ? TRUE : FALSE, NULL);
	gconf_client_set_bool(client, GCONF_PATH "/sound_support",
			 FLAG_ISSET(sndfgs, 0) ? TRUE : FALSE, NULL);

	WriteNetSegment(client);

	g_object_unref(client);
}

/**
 * 深拷贝一份网段数据.
 * @return 网段数据
 */
GSList *ProgramData::CopyNetSegment()
{
	NetSegment *ns, *pns;
	GSList *tlist, *nseg;

	nseg = NULL;
	tlist = netseg;
	while (tlist) {
		pns = (NetSegment *)tlist->data;
		ns = new NetSegment;
		nseg = g_slist_append(nseg, ns);
		ns->startip = g_strdup(pns->startip);
		ns->endip = g_strdup(pns->endip);
		ns->description = g_strdup(pns->description);
		tlist = g_slist_next(tlist);
	}

	return nseg;
}

/**
 * 查询(ipv4)所在网段的描述串.
 * @param ipv4 ipv4
 * @return 描述串
 */
char *ProgramData::FindNetSegDescription(in_addr_t ipv4)
{
	in_addr_t startip, endip;
	NetSegment *pns;
	GSList *tlist;
	char *description;

	ipv4 = ntohl(ipv4);
	description = NULL;
	tlist = netseg;
	while (tlist) {
		pns = (NetSegment *)tlist->data;
		inet_pton(AF_INET, pns->startip, &startip);
		startip = ntohl(startip);
		inet_pton(AF_INET, pns->endip, &endip);
		endip = ntohl(endip);
		ipv4_order(&startip, &endip);
		if (ipv4 >= startip && ipv4 <= endip) {
			description = g_strdup(pns->description);
			break;
		}
		tlist = g_slist_next(tlist);
	}

	return description;
}

/**
 * 读取程序数据.
 */
void ProgramData::ReadProgData()
{
	GConfClient *client;
	GConfValue *value;

	client = gconf_client_get_default();

	if (!(nickname = gconf_client_get_string(client, GCONF_PATH "/nick_name", NULL)))
		nickname = g_strdup(g_get_user_name());
	if (!(mygroup = gconf_client_get_string(client,
		 GCONF_PATH "/belong_group", NULL)))
		mygroup = g_strdup("");
	if (!(myicon = gconf_client_get_string(client, GCONF_PATH "/my_icon", NULL)))
		myicon = g_strdup("icon-tux.png");
	if (!(path = gconf_client_get_string(client, GCONF_PATH "/archive_path", NULL)))
		path = g_strdup(g_get_home_dir());
	if (!(sign = gconf_client_get_string(client, GCONF_PATH "/personal_sign", NULL)))
		sign = g_strdup("");

	if (!(codeset = gconf_client_get_string(client,
		 GCONF_PATH "/candidacy_encode", NULL)))
		codeset = g_strdup(_("utf-16"));
	if (!(encode = gconf_client_get_string(client,
		 GCONF_PATH "/preference_encode", NULL)))
		encode = g_strdup(_("utf-8"));
	if (!(palicon = gconf_client_get_string(client, GCONF_PATH "/pal_icon", NULL)))
		palicon = g_strdup("icon-qq.png");
	if (!(font = gconf_client_get_string(client, GCONF_PATH "/panel_font", NULL)))
		font = g_strdup("Sans Serif 10");
	if (gconf_client_get_bool(client, GCONF_PATH "/hide_startup", NULL))
		FLAG_SET(flags, 6);
	if (gconf_client_get_bool(client, GCONF_PATH "/open_transmission", NULL))
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

	if (!(msgtip = gconf_client_get_string(client, GCONF_PATH "/msg_tip", NULL)))
		msgtip = g_strdup(__SOUND_PATH "/msg.ogg");
	if (!(transtip = gconf_client_get_string(client, GCONF_PATH "/trans_tip", NULL)))
		transtip = g_strdup(__SOUND_PATH "/trans.ogg");
	if ( (value = gconf_client_get(client, GCONF_PATH "/volume_degree", NULL))) {
		volume = gconf_value_get_float(value);
		gconf_value_free(value);
	}
	if ( (value = gconf_client_get(client, GCONF_PATH "/transnd_support", NULL))) {
		if (!gconf_value_get_bool(value))
			FLAG_CLR(sndfgs, 2);
		gconf_value_free(value);
	}
	if ( (value = gconf_client_get(client, GCONF_PATH "/msgsnd_support", NULL))) {
		if (!gconf_value_get_bool(value))
			FLAG_CLR(sndfgs, 1);
		gconf_value_free(value);
	}
	if ( (value = gconf_client_get(client, GCONF_PATH "/sound_support", NULL))) {
		if (!gconf_value_get_bool(value))
			FLAG_CLR(sndfgs, 0);
		gconf_value_free(value);
	}

	ReadNetSegment(client);

	g_object_unref(client);
}

/**
 * 确保头像数据被存放在主题库中.
 */
void ProgramData::CheckIconTheme()
{
	char pathbuf[MAX_PATHLEN];
	GdkPixbuf *pixbuf;

	snprintf(pathbuf, MAX_PATHLEN, __ICON_PATH "/%s", myicon);
	if (access(pathbuf, F_OK) != 0) {
		snprintf(pathbuf, MAX_PATHLEN, "%s" ICON_PATH "/%s",
				 g_get_user_config_dir(), myicon);
		if ( (pixbuf = gdk_pixbuf_new_from_file(pathbuf, NULL))) {
			gtk_icon_theme_add_builtin_icon(myicon, MAX_ICONSIZE, pixbuf);
			g_object_unref(pixbuf);
		}
	}

	snprintf(pathbuf, MAX_PATHLEN, __ICON_PATH "/%s", palicon);
	if (access(pathbuf, F_OK) != 0) {
		snprintf(pathbuf, MAX_PATHLEN, "%s" ICON_PATH "/%s",
				 g_get_user_config_dir(), palicon);
		if ( (pixbuf = gdk_pixbuf_new_from_file(pathbuf, NULL))) {
			gtk_icon_theme_add_builtin_icon(palicon, MAX_ICONSIZE, pixbuf);
			g_object_unref(pixbuf);
		}
	}
}

/**
 * 创建识别URL的正则表达式.
 */
void ProgramData::CreateRegex()
{
	urlregex = g_regex_new(URL_REGEX, GRegexCompileFlags(0),
				 GRegexMatchFlags(0), NULL);
}

/**
 * 创建鼠标光标.
 */
void ProgramData::CreateCursor()
{
	xcursor = gdk_cursor_new(GDK_XTERM);
	lcursor = gdk_cursor_new(GDK_HAND2);
}

/**
 * 创建用于(text-view)的一些通用tag.
 * @note 给这些tag一个"global"标记，表示这些对象是全局共享的
 */
void ProgramData::CreateTagTable()
{
	GtkTextTag *tag;

	table = gtk_text_tag_table_new();

	tag = gtk_text_tag_new("pal-color");
	g_object_set(tag, "foreground", "blue", NULL);
	g_object_set_data(G_OBJECT(tag), "global", GINT_TO_POINTER(TRUE));
	gtk_text_tag_table_add(table, tag);
	g_object_unref(tag);

	tag = gtk_text_tag_new("me-color");
	g_object_set(tag, "foreground", "green", NULL);
	g_object_set_data(G_OBJECT(tag), "global", GINT_TO_POINTER(TRUE));
	gtk_text_tag_table_add(table, tag);
	g_object_unref(tag);

	tag = gtk_text_tag_new("error-color");
	g_object_set(tag, "foreground", "red", NULL);
	g_object_set_data(G_OBJECT(tag), "global", GINT_TO_POINTER(TRUE));
	gtk_text_tag_table_add(table, tag);
	g_object_unref(tag);

	tag = gtk_text_tag_new("sign-words");
	g_object_set(tag, "indent", 10, "foreground", "#1005F0",
				 "font", "Sans Italic 8", NULL);
	g_object_set_data(G_OBJECT(tag), "global", GINT_TO_POINTER(TRUE));
	gtk_text_tag_table_add(table, tag);
	g_object_unref(tag);

	tag = gtk_text_tag_new("url-link");
	g_object_set(tag, "foreground", "blue",
		 "underline", PANGO_UNDERLINE_SINGLE, NULL);
	g_object_set_data(G_OBJECT(tag), "global", GINT_TO_POINTER(TRUE));
	gtk_text_tag_table_add(table, tag);
	g_object_unref(tag);
}

/**
 * 写出网段数据.
 * @param client GConfClient
 */
void ProgramData::WriteNetSegment(GConfClient *client)
{
	NetSegment *pns;
	GSList *list, *tlist;

	list = NULL;
	pthread_mutex_lock(&mutex);
	tlist = netseg;
	while (tlist) {
		pns = (NetSegment *)tlist->data;
		list = g_slist_append(list, pns->startip);
		list = g_slist_append(list, pns->endip);
		list = g_slist_append(list, pns->description ?
				 pns->description : (void*)"");
		tlist = g_slist_next(tlist);
	}
	pthread_mutex_unlock(&mutex);
	gconf_client_set_list(client, GCONF_PATH "/scan_net_segment",
				 GCONF_VALUE_STRING, list, NULL);
	g_slist_free(list);
}

/**
 * 读取网段数据.
 * @param client GConfClient
 */
void ProgramData::ReadNetSegment(GConfClient *client)
{
	NetSegment *ns;
	GSList *list, *tlist;

	tlist = list = gconf_client_get_list(client, GCONF_PATH "/scan_net_segment",
						 GCONF_VALUE_STRING, NULL);
	pthread_mutex_lock(&mutex);
	while (tlist) {
		ns = new NetSegment;
		netseg = g_slist_append(netseg, ns);
		ns->startip = (char *)tlist->data;
		tlist = g_slist_next(tlist);
		ns->endip = (char *)tlist->data;
		tlist = g_slist_next(tlist);
		ns->description = (char *)tlist->data;
		tlist = g_slist_next(tlist);
	}
	pthread_mutex_unlock(&mutex);
	g_slist_free(list);
}
