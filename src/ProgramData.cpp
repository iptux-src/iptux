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
#include "CoreThread.h"
#include "utils.h"

/**
 * 类构造函数.
 */
ProgramData::ProgramData():nickname(NULL), mygroup(NULL),
 myicon(NULL), path(NULL),  sign(NULL), codeset(NULL), encode(NULL),
 palicon(NULL), font(NULL), flags(0), transtip(NULL), msgtip(NULL),
 volume(1.0), sndfgs(~0), netseg(NULL), urlregex(NULL), xcursor(NULL),
 lcursor(NULL), table(NULL), cnxnid(0)
{
        gettimeofday(&timestamp, NULL);
        pthread_mutex_init(&mutex, NULL);
}

/**
 * 类析构函数.
 */
ProgramData::~ProgramData()
{
        GConfClient *client;

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

        for (GSList *tlist = netseg; tlist; tlist = g_slist_next(tlist))
                delete (NetSegment *)tlist->data;
        g_slist_free(netseg);

        if (urlregex)
                g_regex_unref(urlregex);
        if (xcursor)
                gdk_cursor_unref(xcursor);
        if (lcursor)
                gdk_cursor_unref(lcursor);
        if (table)
                g_object_unref(table);

        if (cnxnid > 0) {
                client = gconf_client_get_default();
                gconf_client_notify_remove(client, cnxnid);
                g_object_unref(client);
        }
        pthread_mutex_destroy(&mutex);
}

/**
 * 初始化相关类成员数据.
 */
void ProgramData::InitSublayer()
{
        ReadProgData();
        AddGconfNotify();
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
        gettimeofday(&timestamp, NULL); //更新时间戳

        gconf_client_set_string(client, GCONF_PATH "/nick_name", nickname, NULL);
        gconf_client_set_string(client, GCONF_PATH "/belong_group", mygroup, NULL);
        gconf_client_set_string(client, GCONF_PATH "/my_icon", myicon, NULL);
        gconf_client_set_string(client, GCONF_PATH "/archive_path", path, NULL);
        gconf_client_set_string(client, GCONF_PATH "/personal_sign", sign, NULL);

        gconf_client_set_string(client, GCONF_PATH "/candidacy_encode", codeset, NULL);
        gconf_client_set_string(client, GCONF_PATH "/preference_encode", encode, NULL);
        gconf_client_set_string(client, GCONF_PATH "/pal_icon", palicon, NULL);
        gconf_client_set_string(client, GCONF_PATH "/panel_font", font, NULL);
	gconf_client_set_bool(client, GCONF_PATH "/open-chat",
                         FLAG_ISSET(flags, 7) ? TRUE : FALSE, NULL);
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
        gconf_client_set_bool(client, GCONF_PATH "/transnd_support",
                         FLAG_ISSET(sndfgs, 2) ? TRUE : FALSE, NULL);
        gconf_client_set_bool(client, GCONF_PATH "/msgsnd_support",
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
        if (gconf_client_get_bool(client, GCONF_PATH "/open-chat", NULL))
                FLAG_SET(flags, 7);
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
 * 监视程序配置文件信息数据的变更.
 */
void ProgramData::AddGconfNotify()
{
        GConfClient *client;

        client = gconf_client_get_default();
        gconf_client_add_dir(client, GCONF_PATH, GCONF_CLIENT_PRELOAD_NONE, NULL);
        cnxnid = gconf_client_notify_add(client, GCONF_PATH,
                 GConfClientNotifyFunc(GconfNotifyFunc), this, NULL, NULL);
        g_object_unref(client);
}

/**
 * 确保头像数据被存放在主题库中.
 */
void ProgramData::CheckIconTheme()
{
        char pathbuf[MAX_PATHLEN];
        GdkPixbuf *pixbuf;

        snprintf(pathbuf, MAX_PATHLEN, __PIXMAPS_PATH "/icon/%s", myicon);
        if (access(pathbuf, F_OK) != 0) {
                snprintf(pathbuf, MAX_PATHLEN, "%s" ICON_PATH "/%s",
                                 g_get_user_config_dir(), myicon);
                if ( (pixbuf = gdk_pixbuf_new_from_file(pathbuf, NULL))) {
                        gtk_icon_theme_add_builtin_icon(myicon, MAX_ICONSIZE, pixbuf);
                        g_object_unref(pixbuf);
                }
        }

        snprintf(pathbuf, MAX_PATHLEN, __PIXMAPS_PATH "/icon/%s", palicon);
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

/**
 * 配置文件信息数据变更的响应处理函数.
 * 当本程序写出数据时，程序会自动更新时间戳，所以若当前时间与时间戳间隔太短，
 * 便认为是本程序写出数据导致配置文件信息数据发生了变化，在这种情况下，
 * 响应函数无需理睬数值的变更.\n
 * @param client the GConfClient notifying us.
 * @param cnxnid connection ID from gconf_client_notify_add().
 * @param entry a GConfEntry.
 * @param progdt 程序数据类
 */
void ProgramData::GconfNotifyFunc(GConfClient *client, guint cnxnid,
                                 GConfEntry *entry, ProgramData *progdt)
{
        struct timeval stamp;
        const char *str;
        bool update;

        /* 如果没有值则直接跳出 */
        if (!entry->value)
                return;
        /* 如果间隔太短则直接跳出 */
        gettimeofday(&stamp, NULL);
        if (difftimeval(stamp, progdt->timestamp) < 1.0)
                return;

        /* 匹配键值并修正 */
        update = false; //预设更新标记为假
        if (strcmp(entry->key, GCONF_PATH "/nick_name") == 0) {
                if ( (str = gconf_value_get_string(entry->value))) {
                        g_free(progdt->nickname);
                        progdt->nickname = g_strdup(str);
                        update = true;
                }
        } else if (strcmp(entry->key, GCONF_PATH "/belong_group") == 0) {
                if ( (str = gconf_value_get_string(entry->value))) {
                        g_free(progdt->mygroup);
                        progdt->mygroup = g_strdup(str);
                        update = true;
                }
        } else if (strcmp(entry->key, GCONF_PATH "/my_icon") == 0) {
                if ( (str = gconf_value_get_string(entry->value))) {
                        g_free(progdt->myicon);
                        progdt->myicon = g_strdup(str);
                        update = true;
                }
        } else if (strcmp(entry->key, GCONF_PATH "/archive_path") == 0) {
                if ( (str = gconf_value_get_string(entry->value))) {
                        g_free(progdt->path);
                        progdt->path = g_strdup(str);
                }
        } else if (strcmp(entry->key, GCONF_PATH "/personal_sign") == 0) {
                if ( (str = gconf_value_get_string(entry->value))) {
                        g_free(progdt->sign);
                        progdt->sign = g_strdup(str);
                        update = true;
                }
        } else if (strcmp(entry->key, GCONF_PATH "/candidacy_encode") == 0) {
                if ( (str = gconf_value_get_string(entry->value))) {
                        g_free(progdt->codeset);
                        progdt->codeset = g_strdup(str);
                }
        } else if (strcmp(entry->key, GCONF_PATH "/preference_encode") == 0) {
                if ( (str = gconf_value_get_string(entry->value))) {
                        g_free(progdt->encode);
                        progdt->encode = g_strdup(str);
                }
        } else if (strcmp(entry->key, GCONF_PATH "/pal_icon") == 0) {
                if ( (str = gconf_value_get_string(entry->value))) {
                        g_free(progdt->palicon);
                        progdt->palicon = g_strdup(str);
                }
        } else if (strcmp(entry->key, GCONF_PATH "/panel_font") == 0) {
                if ( (str = gconf_value_get_string(entry->value))) {
                        g_free(progdt->font);
                        progdt->font = g_strdup(str);
                }
        } else if (strcmp(entry->key, GCONF_PATH "/hide_startup") == 0) {
                if (gconf_value_get_bool(entry->value))
                        FLAG_SET(progdt->flags, 6);
                else
                        FLAG_CLR(progdt->flags, 6);
        } else if (strcmp(entry->key, GCONF_PATH "/open_transmission") == 0) {
                if (gconf_value_get_bool(entry->value))
                        FLAG_SET(progdt->flags, 5);
                else
                        FLAG_CLR(progdt->flags, 5);
        } else if (strcmp(entry->key, GCONF_PATH "/use_enter_key") == 0) {
                if (gconf_value_get_bool(entry->value))
                        FLAG_SET(progdt->flags, 4);
                else
                        FLAG_CLR(progdt->flags, 4);
        } else if (strcmp(entry->key, GCONF_PATH "/clearup_history") == 0) {
                if (gconf_value_get_bool(entry->value))
                        FLAG_SET(progdt->flags, 3);
                else
                        FLAG_CLR(progdt->flags, 3);
        } else if (strcmp(entry->key, GCONF_PATH "/record_log") == 0) {
                if (gconf_value_get_bool(entry->value))
                        FLAG_SET(progdt->flags, 2);
                else
                        FLAG_CLR(progdt->flags, 2);
        } else if (strcmp(entry->key, GCONF_PATH "/open_blacklist") == 0) {
                if (gconf_value_get_bool(entry->value))
                        FLAG_SET(progdt->flags, 1);
                else
                        FLAG_CLR(progdt->flags, 1);
        } else if (strcmp(entry->key, GCONF_PATH "/proof_shared") == 0) {
                if (gconf_value_get_bool(entry->value))
                        FLAG_SET(progdt->flags, 0);
                else
                        FLAG_CLR(progdt->flags, 0);
        } else if (strcmp(entry->key, GCONF_PATH "/trans_tip") == 0) {
                if ( (str = gconf_value_get_string(entry->value))) {
                        g_free(progdt->transtip);
                        progdt->transtip = g_strdup(str);
                }
        } else if (strcmp(entry->key, GCONF_PATH "/msg_tip") == 0) {
                if ( (str = gconf_value_get_string(entry->value))) {
                        g_free(progdt->transtip);
                        progdt->transtip = g_strdup(str);
                }
        } else if (strcmp(entry->key, GCONF_PATH "/volume_degree") == 0) {
                progdt->volume = gconf_value_get_float(entry->value);
        } else if (strcmp(entry->key, GCONF_PATH "/transnd_support") == 0) {
                if (gconf_value_get_bool(entry->value))
                        FLAG_SET(progdt->sndfgs, 2);
                else
                        FLAG_CLR(progdt->sndfgs, 2);
        } else if (strcmp(entry->key, GCONF_PATH "/msgsnd_support") == 0) {
                if (gconf_value_get_bool(entry->value))
                        FLAG_SET(progdt->sndfgs, 1);
                else
                        FLAG_CLR(progdt->sndfgs, 1);
        } else if (strcmp(entry->key, GCONF_PATH "/sound_support") == 0) {
                if (gconf_value_get_bool(entry->value))
                        FLAG_SET(progdt->sndfgs, 0);
                else
                        FLAG_CLR(progdt->sndfgs, 0);
        }

        /* 如果需要更新则调用更新处理函数 */
        if (update)
                CoreThread::UpdateMyInfo();
}
