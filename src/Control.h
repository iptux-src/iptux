//
// C++ Interface: Control
//
// Description:与iptux相关的程序数据
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CONTROL_H
#define CONTROL_H

#include "face.h"
#include "sys.h"
#include "net.h"

class Control {
 public:
	Control();
	~Control();

	void InitSelf();
	void WriteControl();
	void AdjustMemory();
	GSList *CopyNetSegment();
	char *FindNetSegDescribe(in_addr_t ipv4);

	char *myname;		//昵称
	char *mygroup;	//组，mygroup != NULL
	char *myicon;		//自身头像
	char *path;		//文件存放路径
	char *sign;		//个性签名

	char *encode;		//默认网络编码
	char *palicon;		//默认头像
	char *font;		//字体
	uint8_t flags;		//5 内存:4 enter:3 清除历史:2 日志记录:1 黑名单:0 共享过滤

	char *msgtip;
	char *transtip;
	double volume;
	uint8_t sndfgs;		//2 传输:1 消息:0 声音

	GSList *netseg;		//通知登录IP段，netseg.describe != NULL
	bool dirty;		//重写标记

	GtkTextTagTable *table;
	GSList *iconlist;		//系统头像链表，只能初始化而不能修改，因此不必加锁保护
	gfloat pix;

	pthread_mutex_t mutex;

public:
    static gboolean hovering_over_link;
    static GdkCursor *hand_cursor;
    static GdkCursor *regular_cursor;
public:
    static void 
    screen_show_url(GtkWidget *text_view, const gchar *url);
    static void
    follow_if_link (GtkWidget   *text_view, 
                    GtkTextIter *iter);
    static gboolean
    event_after (GtkWidget *text_view,
                 GdkEvent  *ev);
    static void
    set_cursor_if_appropriate (GtkTextView    *text_view,
                               gint            x,
                               gint            y);
    static gboolean
    motion_notify_event (GtkWidget      *text_view,
                         GdkEventMotion *event);
    static gboolean
    visibility_notify_event (GtkWidget          *text_view,
                             GdkEventVisibility *event);
 private:
	void ReadControl();
	void CreateTagTable();
	void GetSysIcon();
	void GetRatio_PixMm();

	void UpdateNetSegment(GConfClient * client, bool direc);
};

#endif
