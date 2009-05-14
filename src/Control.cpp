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
#include "output.h"

gboolean Control::hovering_over_link = false;
GdkCursor* Control::hand_cursor = NULL;
GdkCursor* Control::regular_cursor = NULL;


Control::Control(): myname(NULL), mygroup(NULL), myicon(NULL),
path(NULL),  sign(NULL), encode(NULL), palicon(NULL),font(NULL),
flags(0), msgtip(NULL), transtip(NULL), volume(1.0), sndfgs(~0),
netseg(NULL), dirty(false), table(NULL), iconlist(NULL), pix(3.4)
{
	pthread_mutex_init(&mutex, NULL);

//    hovering_over_link = false;
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

	free(myname);
	free(mygroup);
	free(myicon);
	free(path);
	free(sign);
	free(encode);
	free(palicon);
	free(font);
	free(msgtip);
	free(transtip);

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
	gconf_client_set_string(client, GCONF_PATH "/nick_name", myname, NULL);
	gconf_client_set_string(client, GCONF_PATH "/belong_group", mygroup, NULL);
	gconf_client_set_string(client, GCONF_PATH "/self_icon", myicon, NULL);
	gconf_client_set_string(client, GCONF_PATH "/save_path", path, NULL);
	gconf_client_set_string(client, GCONF_PATH "/personal_sign", sign, NULL);

	gconf_client_set_string(client, GCONF_PATH "/net_encode", encode, NULL);
	gconf_client_set_string(client, GCONF_PATH "/pal_icon", palicon, NULL);
	gconf_client_set_string(client, GCONF_PATH "/panel_font", font, NULL);
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

	gconf_client_set_string(client, GCONF_PATH "/msg_tip", msgtip, NULL);
	gconf_client_set_string(client, GCONF_PATH "/trans_tip", transtip, NULL);
	gconf_client_set_float(client, GCONF_PATH "/volume_degree", volume, NULL);
	gconf_client_set_bool(client, GCONF_PATH "/msgsnd_support",
			      FLAG_ISSET(sndfgs, 2) ? TRUE : FALSE, NULL);
	gconf_client_set_bool(client, GCONF_PATH "/transnd_support",
			      FLAG_ISSET(sndfgs, 1) ? TRUE : FALSE, NULL);
	gconf_client_set_bool(client, GCONF_PATH "/sound_support",
			      FLAG_ISSET(sndfgs, 0) ? TRUE : FALSE, NULL);

	UpdateNetSegment(client, true);
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
		ipv4_order(&ip1, &ip2);
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
	GConfValue *value;

	client = gconf_client_get_default();
	if (!(myname =
	      gconf_client_get_string(client, GCONF_PATH "/nick_name", NULL)))
		myname = Strdup(g_get_user_name());
	if (!(mygroup =
	      gconf_client_get_string(client, GCONF_PATH "/belong_group", NULL)))
		mygroup = Strdup("");
	if (!(myicon =
	      gconf_client_get_string(client, GCONF_PATH "/self_icon", NULL)))
		myicon = Strdup(__ICON_PATH "/tux.png");
	if (!(path =
	      gconf_client_get_string(client, GCONF_PATH "/save_path", NULL)))
		path = Strdup(g_get_home_dir());
	if (!(sign =
	      gconf_client_get_string(client, GCONF_PATH "/personal_sign", NULL)))
		sign = Strdup("");

	if (!(encode =
	      gconf_client_get_string(client, GCONF_PATH "/net_encode", NULL)))
		encode = Strdup(_("UTF-8"));
	if (!(palicon =
	      gconf_client_get_string(client, GCONF_PATH "/pal_icon", NULL)))
		palicon = Strdup(__ICON_PATH "/qq.png");
	if (!(font =
	      gconf_client_get_string(client, GCONF_PATH "/panel_font", NULL)))
		font = Strdup("Sans Italic 10");
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

	if (!(msgtip =
	      gconf_client_get_string(client, GCONF_PATH "/msg_tip", NULL)))
		msgtip = Strdup(__SOUND_PATH "/msg.ogg");
	if (!(transtip =
	      gconf_client_get_string(client, GCONF_PATH "/trans_tip", NULL)))
		transtip = Strdup(__SOUND_PATH "/trans.ogg");
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

	UpdateNetSegment(client, false);
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
	g_object_set(tag, "foreground", "#007500", NULL);
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

	mf.chdir(__ICON_PATH);
	if (!(dir = mf.opendir()))
		return;
	while ( (dirt = readdir(dir))) {
		if (strcmp(dirt->d_name, ".") == 0
		    || strcmp(dirt->d_name, "..") == 0)
			continue;
		snprintf(path, MAX_PATHBUF, __ICON_PATH "/%s", dirt->d_name);
		iconlist = g_slist_append(iconlist,
			  new SysIcon(Strdup(path), NULL));		//延迟到守护线程完成所有工作
	}
	closedir(dir);

	/* 当使用好友自定义头像时，进一步节俭内存 */
	if (strncmp(palicon, __ICON_PATH, strlen(__ICON_PATH)) != 0)
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

/*
 *
 *
 *以下为url处理函数
 *
 */


void 
Control::screen_show_url(GtkWidget *text_view, const gchar *url)
{
    GError *error = NULL;
    GdkScreen *screen;

    if (gtk_widget_has_screen (text_view))
        screen = gtk_widget_get_screen (text_view);
    else
        screen = gdk_screen_get_default ();

    gtk_show_uri (screen, url,
               gtk_get_current_event_time (),
               &error);

    if (error != NULL)
    {
        g_printerr ("Error showing url: %s\n", error->message);
        pop_warning(NULL, NULL, _("Getting Default Browser Error. \n%s"),
                                        error->message);
        g_error_free (error);
    }
}

void
Control::follow_if_link (GtkWidget   *text_view, 
                GtkTextIter *iter)
{
    GSList *tags = NULL, *tagp = NULL;

    tags = gtk_text_iter_get_tags (iter);
    for (tagp = tags;  tagp != NULL;  tagp = tagp->next)
    {
        GtkTextTag *tag = (GtkTextTag *)tagp->data;
        gchar *url = (gchar *)g_object_get_data (G_OBJECT (tag), "url");

        if (url != NULL) 
        {
    //        g_print("%s\n", url);
            screen_show_url(text_view, url);
            break;
        }
    }

    if (tags) 
        g_slist_free (tags);
}


/* Links can also be activated by clicking.
 */
gboolean
Control::event_after (GtkWidget *text_view,
             GdkEvent  *ev)
{
  GtkTextIter start, end, iter;
  GtkTextBuffer *buffer;
  GdkEventButton *event;
  gint x, y;

  if (ev->type != GDK_BUTTON_RELEASE)
    return FALSE;

  event = (GdkEventButton *)ev;

  if (event->button != 1)
    return FALSE;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

  // we shouldn't follow a link if the user has selected something 
  gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
  if (gtk_text_iter_get_offset (&start) != gtk_text_iter_get_offset (&end))
    return FALSE;

  gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view), 
                                         GTK_TEXT_WINDOW_WIDGET,
                                         event->x, event->y, &x, &y);

  gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (text_view), &iter, x, y);

  follow_if_link (text_view, &iter);

  return FALSE;
}


/* Looks at all tags covering the position (x, y) in the text view, 
 * and if one of them is a link, change the cursor to the "hands" cursor
 * typically used by web browsers.
 */
void
Control::set_cursor_if_appropriate (GtkTextView    *text_view,
                           gint            x,
                           gint            y)
{
    GSList *tags = NULL, *tagp = NULL;
    GtkTextIter iter;
    gboolean hovering = FALSE;

    gtk_text_view_get_iter_at_location (text_view, &iter, x, y);
  
    tags = gtk_text_iter_get_tags (&iter);
    for (tagp = tags;  tagp != NULL;  tagp = tagp->next)
    {
        GtkTextTag *tag = (GtkTextTag *)tagp->data;
        gchar *url = (gchar *)g_object_get_data (G_OBJECT (tag), "url");

        if (url != NULL) 
        {
            hovering = TRUE;
            break;
        }
    }

    if (hovering != hovering_over_link)
    {
        hovering_over_link = hovering;

        if (hovering_over_link)
            gdk_window_set_cursor (gtk_text_view_get_window (text_view, GTK_TEXT_WINDOW_TEXT), hand_cursor);
        else
            gdk_window_set_cursor (gtk_text_view_get_window (text_view, GTK_TEXT_WINDOW_TEXT), regular_cursor);
    }

    if (tags) 
        g_slist_free (tags);
}

/* Update the cursor image if the pointer moved. 
 */
gboolean
Control::motion_notify_event (GtkWidget      *text_view,
                     GdkEventMotion *event)
{
  gint x, y;

  gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view), 
                                         GTK_TEXT_WINDOW_WIDGET,
                                         event->x, event->y, &x, &y);

  set_cursor_if_appropriate (GTK_TEXT_VIEW (text_view), x, y);

  gdk_window_get_pointer (text_view->window, NULL, NULL, NULL);
  return FALSE;
}

/* Also update the cursor image if the window becomes visible
 * (e.g. when a window covering it got iconified).
 */
gboolean
Control::visibility_notify_event (GtkWidget          *text_view,
                         GdkEventVisibility *event)
{
  gint wx, wy, bx, by;
  
  gdk_window_get_pointer (text_view->window, &wx, &wy, NULL);
  
  gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view), 
                                         GTK_TEXT_WINDOW_WIDGET,
                                         wx, wy, &bx, &by);

  set_cursor_if_appropriate (GTK_TEXT_VIEW (text_view), bx, by);

  return FALSE;
}


