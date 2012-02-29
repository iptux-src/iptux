//
// C++ Implementation: CoreThread
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "CoreThread.h"
#include "ProgramData.h"
#include "MainWindow.h"
#include "LogSystem.h"
#include "UdpData.h"
#include "TcpData.h"
#include "Command.h"
#include "output.h"
#include "support.h"
#include "utils.h"
extern ProgramData progdt;
extern CoreThread cthrd;
extern MainWindow mwin;
extern LogSystem lgsys;

/**
 * 类构造函数.
 */
CoreThread::CoreThread():tcpsock(-1), udpsock(-1), server(true),
 pallist(NULL), rgllist(NULL), sgmlist(NULL), grplist(NULL), brdlist(NULL),
 blacklist(NULL), pbn(1), prn(MAX_SHAREDFILE), pblist(NULL), prlist(NULL),
 ecsList(NULL),passwd(NULL)
{
        g_queue_init(&msgline);
        pthread_mutex_init(&mutex, NULL);
}

/**
 * 类析构函数.
 */
CoreThread::~CoreThread()
{
        ClearSublayer();
}

/**
 * 程序核心入口，主要任务服务将在此开启.
 */
void CoreThread::CoreThreadEntry()
{
        pthread_t pid;

        /* 初始化底层数据 */
        InitSublayer();
        /* 开启UDP监听服务 */
        pthread_create(&pid, NULL, ThreadFunc(RecvUdpData), this);
        pthread_detach(pid);
        /* 开启TCP监听服务 */
        pthread_create(&pid, NULL, ThreadFunc(RecvTcpData), this);
        pthread_detach(pid);
        /* 定时扫描处理程序内部任务 */
        timerid = gdk_threads_add_timeout(500, GSourceFunc(WatchCoreStatus), this);
        /* 通知所有计算机本大爷上线啦 */
        pthread_create(&pid, NULL, ThreadFunc(SendNotifyToAll), this);
}

/**
 * 写出共享文件数据.
 * @note 与可能修改链表的代码段串行执行，没有加锁的必要
 */
void CoreThread::WriteSharedData()
{
        GConfClient *client;
        GSList *list, *tlist;

        list = NULL;
        /* 获取共享文件链表 */
        tlist = pblist;
        while (tlist) {
                list = g_slist_append(list, ((FileInfo *)tlist->data)->filepath);
                tlist = g_slist_next(tlist);
        }
        /* 写出数据 */
        client = gconf_client_get_default();
        gconf_client_set_list(client, GCONF_PATH "/shared_file_list",
                                         GCONF_VALUE_STRING, list, NULL);
        if (passwd)
                gconf_client_set_string(client, GCONF_PATH "/access_shared_limit",
                                                                 passwd, NULL);
        g_object_unref(client);
        /* 释放链表 */
        g_slist_free(list);
}

/**
 * 获取好友链表.
 * @return 好友链表
 */
GSList *CoreThread::GetPalList()
{
        return pallist;
}

/**
 * 获取锁.
 * @return 锁
 */
pthread_mutex_t *CoreThread::GetMutex()
{
        return &mutex;
}

/**
 * 插入消息(UI线程安全).
 * @param para 消息参数封装包
 * @note 消息数据必须使用utf8编码
 * @note (para->pal)不可为null
 * @note 请不要关心函数内部实现，你只需要按照要求封装消息数据，然后扔给本函数处理就可以了，
 * 它会想办法将消息按照你所期望的格式插入到你所期望的TextBuffer，否则请发送Bug报告
 */
void CoreThread::InsertMessage(MsgPara *para)
{
        GroupInfo *grpinf;
        SessionAbstract *session;

        /* 启用UI线程安全保护 */
        gdk_threads_enter();

        /* 获取群组信息 */
        switch (para->btype) {
        case GROUP_BELONG_TYPE_REGULAR:
                grpinf = GetPalRegularItem(para->pal);
                break;
        case GROUP_BELONG_TYPE_SEGMENT:
                grpinf = GetPalSegmentItem(para->pal);
                break;
        case GROUP_BELONG_TYPE_GROUP:
                grpinf = GetPalGroupItem(para->pal);
                break;
        case GROUP_BELONG_TYPE_BROADCAST:
                grpinf = GetPalBroadcastItem(para->pal);
                break;
        default:
                grpinf = NULL;
                break;
        }

        /* 如果群组存在则插入消息 */
        /* 群组不存在是编程上的错误，请发送Bug报告 */
        if (grpinf) {
                InsertMsgToGroupInfoItem(grpinf, para);
                if (grpinf->dialog) {
                        session = (SessionAbstract *)g_object_get_data(G_OBJECT(
                                                 grpinf->dialog), "session-class");
                        session->OnNewMessageComing();
                }
        }

        /* 离开UI操作处理 */
        gdk_threads_leave();
}

/**
 * 插入消息到群组消息缓冲区(非UI线程安全).
 * @param grpinf 群组信息
 * @param para 消息参数
 */
void CoreThread::InsertMsgToGroupInfoItem(GroupInfo *grpinf, MsgPara *para)
{
        GtkTextIter iter;
        GSList *tlist;
        gchar *data;

        tlist = para->dtlist;
        while (tlist) {
                data = ((ChipData *)tlist->data)->data;
                switch (((ChipData *)tlist->data)->type) {
                case MESSAGE_CONTENT_TYPE_STRING:
                        InsertHeaderToBuffer(grpinf->buffer, para);
                        gtk_text_buffer_get_end_iter(grpinf->buffer, &iter);
                        gtk_text_buffer_insert(grpinf->buffer, &iter, "\n", -1);
                        InsertStringToBuffer(grpinf->buffer, data);
                        gtk_text_buffer_get_end_iter(grpinf->buffer, &iter);
                        gtk_text_buffer_insert(grpinf->buffer, &iter, "\n", -1);
                        lgsys.CommunicateLog(para, "[STRING]%s", data);
                        break;
                case MESSAGE_CONTENT_TYPE_PICTURE:
                        InsertPixbufToBuffer(grpinf->buffer, data);
                        lgsys.CommunicateLog(para, "[PICTURE]%s", data);
                        break;
                default:
                        break;
                }
                tlist = g_slist_next(tlist);
        }
}

/**
 * 向局域网内所有计算机发送上线信息.
 * @param pcthrd 核心类
 */
void CoreThread::SendNotifyToAll(CoreThread *pcthrd)
{
        Command cmd;

        cmd.BroadCast(pcthrd->udpsock);
        cmd.DialUp(pcthrd->udpsock);
}

/**
 * 向好友发送iptux特有的数据.
 * @param pal class PalInfo
 */
void CoreThread::SendFeatureData(PalInfo *pal)
{
        Command cmd;
        char path[MAX_PATHLEN];
        const gchar *env;
        int sock;

        if (*progdt.sign != '\0')
                cmd.SendMySign(cthrd.udpsock, pal);
        env = g_get_user_config_dir();
        snprintf(path, MAX_PATHLEN, "%s" ICON_PATH "/%s", env, progdt.myicon);
        if (access(path, F_OK) == 0)
                cmd.SendMyIcon(cthrd.udpsock, pal);
        snprintf(path, MAX_PATHLEN, "%s" PHOTO_PATH "/photo", env);
        if (access(path, F_OK) == 0) {
                if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
                        pop_error(_("Fatal Error!!\nFailed to create new socket!\n%s"),
                                                                 strerror(errno));
                        exit(1);
                }
                cmd.SendSublayer(sock, pal, IPTUX_PHOTOPICOPT, path);
                close(sock);
        }
}

/**
 * 发送通告本计算机下线的信息.
 * @param pal class PalInfo
 */
void CoreThread::SendBroadcastExit(PalInfo *pal)
{
        Command cmd;

        cmd.SendExit(cthrd.udpsock, pal);
}

/**
 * 更新本大爷的个人信息.
 */
void CoreThread::UpdateMyInfo()
{
        Command cmd;
        pthread_t pid;
        PalInfo *pal;
        GSList *tlist;

        pthread_mutex_lock(&cthrd.mutex);
        tlist = cthrd.pallist;
        while (tlist) {
                pal = (PalInfo *)tlist->data;
                if (FLAG_ISSET(pal->flags, 1))
                        cmd.SendAbsence(cthrd.udpsock, pal);
                if (FLAG_ISSET(pal->flags, 1) && FLAG_ISSET(pal->flags, 0)) {
                        pthread_create(&pid, NULL, ThreadFunc(SendFeatureData), pal);
                        pthread_detach(pid);
                }
                tlist = g_slist_next(tlist);
        }
        pthread_mutex_unlock(&cthrd.mutex);
}

/**
 * 从好友链表中移除所有好友数据(非UI线程安全).
 * @note 鉴于好友链表成员不能被删除，所以将成员改为下线标记即可
 */
void CoreThread::ClearAllPalFromList()
{
        SessionAbstract *session;
        GroupInfo *grpinf;
        PalInfo *pal;
        GSList *tlist;

        /* 清除所有好友的在线标志 */
        tlist = pallist;
        while (tlist) {
                pal = (PalInfo *)tlist->data;
                FLAG_CLR(pal->flags, 1);
                tlist = g_slist_next(tlist);
        }

        /* 清空常规模式下所有群组的成员 */
        tlist = rgllist;
        while (tlist) {
                grpinf = (GroupInfo *)tlist->data;
                g_slist_free(grpinf->member);
                grpinf->member = NULL;
                if (grpinf->dialog) {
                        session = (SessionAbstract *)g_object_get_data(G_OBJECT(
                                                 grpinf->dialog), "session-class");
                        session->ClearAllPalData();
                }
                tlist = g_slist_next(tlist);
        }
        /* 清空网段模式下所有群组的成员 */
        tlist = sgmlist;
        while (tlist) {
                grpinf = (GroupInfo *)tlist->data;
                g_slist_free(grpinf->member);
                grpinf->member = NULL;
                if (grpinf->dialog) {
                        session = (SessionAbstract *)g_object_get_data(G_OBJECT(
                                                 grpinf->dialog), "session-class");
                        session->ClearAllPalData();
                }
                tlist = g_slist_next(tlist);
        }
        /* 清空分组模式下所有群组的成员 */
        tlist = grplist;
        while (tlist) {
                grpinf = (GroupInfo *)tlist->data;
                g_slist_free(grpinf->member);
                grpinf->member = NULL;
                if (grpinf->dialog) {
                        session = (SessionAbstract *)g_object_get_data(G_OBJECT(
                                                 grpinf->dialog), "session-class");
                        session->ClearAllPalData();
                }
                tlist = g_slist_next(tlist);
        }
        /* 清空广播模式下所有群组的成员 */
        tlist = brdlist;
        while (tlist) {
                grpinf = (GroupInfo *)tlist->data;
                g_slist_free(grpinf->member);
                grpinf->member = NULL;
                if (grpinf->dialog) {
                        session = (SessionAbstract *)g_object_get_data(G_OBJECT(
                                                 grpinf->dialog), "session-class");
                        session->ClearAllPalData();
                }
                tlist = g_slist_next(tlist);
        }
}

/**
 * 从好友链表中获取指定的好友信息数据.
 * @param ipv4 ipv4
 * @return 好友信息数据
 */
PalInfo *CoreThread::GetPalFromList(in_addr_t ipv4)
{
        GSList *tlist;

        tlist = pallist;
        while (tlist) {
                if (((PalInfo *)tlist->data)->ipv4 == ipv4)
                        break;
                tlist = g_slist_next(tlist);
        }

        return (PalInfo *)(tlist ? tlist->data : NULL);
}

/**
 * 查询好友链表中是否已经存在此IP地址的信息数据.
 * @param ipv4 ipv4
 * @return 存在与否
 */
bool CoreThread::ListContainPal(in_addr_t ipv4)
{
        GSList *tlist;

        tlist = pallist;
        while (tlist) {
                if (((PalInfo *)tlist->data)->ipv4 == ipv4)
                        break;
                tlist = g_slist_next(tlist);
        }

        return tlist;
}

/**
 * 从好友链表中删除指定的好友信息数据(非UI线程安全).
 * @param ipv4 ipv4
 * @note 鉴于好友链表成员不能被删除，所以将成员改为下线标记即可；
 * 鉴于群组中只能包含在线的好友，所以若某群组中包含了此好友，则必须从此群组中删除此好友
 */
void CoreThread::DelPalFromList(in_addr_t ipv4)
{
        PalInfo *pal;
        GroupInfo *grpinf;

        /* 获取好友信息数据，并将其置为下线状态 */
        if (!(pal = GetPalFromList(ipv4)))
                return;
        FLAG_CLR(pal->flags, 1);

        /* 从群组中移除好友 */
        if ( (grpinf = GetPalRegularItem(pal)))
                DelPalFromGroupInfoItem(grpinf, pal);
        if ( (grpinf = GetPalSegmentItem(pal)))
                DelPalFromGroupInfoItem(grpinf, pal);
        if ( (grpinf = GetPalGroupItem(pal)))
                DelPalFromGroupInfoItem(grpinf, pal);
        if ( (grpinf = GetPalBroadcastItem(pal)))
                DelPalFromGroupInfoItem(grpinf, pal);
}

/**
 * 通告指定的好友信息数据已经被更新(非UI线程安全).
 * @param ipv4 ipv4
 * @note 什么时候会用到？1、好友更新个人资料；2、好友下线后又上线了
 * @note 鉴于群组中必须包含所有属于自己的成员，移除不属于自己的成员，
 * 所以好友信息更新后应该重新调整群组成员；
 * @note 群组中被更新的成员信息也应该在界面上做出相应更新
 */
void CoreThread::UpdatePalToList(in_addr_t ipv4)
{
        PalInfo *pal;
        GroupInfo *grpinf;
        SessionAbstract *session;

        /* 如果好友链表中不存在此好友，则视为程序设计出错 */
        if (!(pal = GetPalFromList(ipv4)))
                return;
        FLAG_SET(pal->flags, 1);

        /* 更新好友所在的群组，以及它在UI上的信息 */
        /*/* 更新常规模式下的群组 */
        if ( (grpinf = GetPalRegularItem(pal))) {
                if (!g_slist_find(grpinf->member, pal)) {
                        AttachPalToGroupInfoItem(grpinf, pal);
                } else if (grpinf->dialog) {
                        session = (SessionAbstract *)g_object_get_data(G_OBJECT(
                                                 grpinf->dialog), "session-class");
                        session->UpdatePalData(pal);
                }
        } else {
                if (!(grpinf = GetPalRegularItem(pal)))
                        grpinf = AttachPalRegularItem(pal);
                AttachPalToGroupInfoItem(grpinf, pal);
        }
        /*/* 更新网段模式下的群组 */
        if ( (grpinf = GetPalSegmentItem(pal))) {
                if (!g_slist_find(grpinf->member, pal)) {
                        AttachPalToGroupInfoItem(grpinf, pal);
                } else if (grpinf->dialog) {
                        session = (SessionAbstract *)g_object_get_data(G_OBJECT(
                                                 grpinf->dialog), "session-class");
                        session->UpdatePalData(pal);
                }
        } else {
                if (!(grpinf = GetPalSegmentItem(pal)))
                        grpinf = AttachPalSegmentItem(pal);
                AttachPalToGroupInfoItem(grpinf, pal);
        }
        /*/* 更新分组模式下的群组 */
        if ( (grpinf = GetPalPrevGroupItem(pal))) {
                if (!pal->group || strcmp(grpinf->name, pal->group) != 0) {
                        DelPalFromGroupInfoItem(grpinf, pal);
                        if (!(grpinf = GetPalGroupItem(pal)))
                                grpinf = AttachPalGroupItem(pal);
                        AttachPalToGroupInfoItem(grpinf, pal);
                } else if (grpinf->dialog) {
                        session = (SessionAbstract *)g_object_get_data(G_OBJECT(
                                                 grpinf->dialog), "session-class");
                        session->UpdatePalData(pal);
                }
        } else {
                if (!(grpinf = GetPalGroupItem(pal)))
                        grpinf = AttachPalGroupItem(pal);
                AttachPalToGroupInfoItem(grpinf, pal);
        }
        /*/* 更新广播模式下的群组 */
        if ( (grpinf = GetPalBroadcastItem(pal))) {
                if (!g_slist_find(grpinf->member, pal)) {
                        AttachPalToGroupInfoItem(grpinf, pal);
                } else if (grpinf->dialog) {
                        session = (SessionAbstract *)g_object_get_data(G_OBJECT(
                                                 grpinf->dialog), "session-class");
                        session->UpdatePalData(pal);
                }
        } else {
                if (!(grpinf = GetPalBroadcastItem(pal)))
                        grpinf = AttachPalBroadcastItem(pal);
                AttachPalToGroupInfoItem(grpinf, pal);
        }
}

/**
 * 将好友信息数据加入到好友链表(非UI线程安全).
 * @param pal class PalInfo
 * @note 鉴于在线的好友必须被分配到它所属的群组，所以加入好友到好友链表的同时
 * 也应该分配好友到相应的群组
 */
void CoreThread::AttachPalToList(PalInfo *pal)
{
        GroupInfo *grpinf;

        /* 将好友加入到好友链表 */
        pallist = g_slist_append(pallist, pal);
        FLAG_SET(pal->flags, 1);

        /* 将好友加入到相应的群组 */
        if (!(grpinf = GetPalRegularItem(pal)))
                grpinf = AttachPalRegularItem(pal);
        AttachPalToGroupInfoItem(grpinf, pal);
        if (!(grpinf = GetPalSegmentItem(pal)))
                grpinf = AttachPalSegmentItem(pal);
        AttachPalToGroupInfoItem(grpinf, pal);
        if (!(grpinf = GetPalGroupItem(pal)))
                grpinf = AttachPalGroupItem(pal);
        AttachPalToGroupInfoItem(grpinf, pal);
        if (!(grpinf = GetPalBroadcastItem(pal)))
                grpinf = AttachPalBroadcastItem(pal);
        AttachPalToGroupInfoItem(grpinf, pal);
}

/**
 * 获取(pal)在常规模式下的群组信息.
 * @param pal class PalInfo
 * @return 群组信息
 */
GroupInfo *CoreThread::GetPalRegularItem(PalInfo *pal)
{
        GSList *tlist;

        tlist = rgllist;
        while (tlist) {
                if (((GroupInfo *)tlist->data)->grpid == pal->ipv4)
                        break;
                tlist = g_slist_next(tlist);
        }

        return (GroupInfo *)(tlist ? tlist->data : NULL);
}

/**
 * 获取(pal)在网段模式下的群组信息.
 * @param pal class PalInfo
 * @return 群组信息
 */
GroupInfo *CoreThread::GetPalSegmentItem(PalInfo *pal)
{
        GSList *tlist;
        char *name;
        GQuark grpid;

        /* 获取局域网网段ID */
        name = ipv4_get_lan_name(pal->ipv4);
        grpid = g_quark_from_string(name ? name : _("Others"));
        g_free(name);

        tlist = sgmlist;
        while (tlist) {
                if (((GroupInfo *)tlist->data)->grpid == grpid)
                        break;
                tlist = g_slist_next(tlist);
        }

        return (GroupInfo *)(tlist ? tlist->data : NULL);
}

/**
 * 获取(pal)在分组模式下的群组信息.
 * @param pal class PalInfo
 * @return 群组信息
 */
GroupInfo *CoreThread::GetPalGroupItem(PalInfo *pal)
{
        GSList *tlist;
        GQuark grpid;

        /* 获取组ID */
        NO_OPERATION_C
        grpid = g_quark_from_string(pal->group ? pal->group : _("Others"));

        tlist = grplist;
        while (tlist) {
                if (((GroupInfo *)tlist->data)->grpid == grpid)
                        break;
                tlist = g_slist_next(tlist);
        }

        return (GroupInfo *)(tlist ? tlist->data : NULL);
}

/**
 * 获取(pal)在广播模式下的群组信息.
 * @param pal class PalInfo
 * @return 群组信息
 */
GroupInfo *CoreThread::GetPalBroadcastItem(PalInfo *pal)
{
        return (GroupInfo *)(brdlist ? brdlist->data : NULL);
}

/**
 * 黑名单链表中是否包含此项.
 * @param ipv4 ipv4
 * @return 是否包含
 */
bool CoreThread::BlacklistContainItem(in_addr_t ipv4)
{
        return g_slist_find(blacklist, GUINT_TO_POINTER(ipv4));
}

/**
 * 加入此项到黑名单链表.
 * @param ipv4 ipv4
 */
void CoreThread::AttachItemToBlacklist(in_addr_t ipv4)
{
        blacklist = g_slist_append(blacklist, GUINT_TO_POINTER(ipv4));
}

/**
 * 清空黑名单链表.
 */
void CoreThread::ClearBlacklist()
{
        g_slist_free(blacklist);
        blacklist = NULL;
}

/**
 * 获取消息队列项总数.
 * @return 项数
 */
guint CoreThread::GetMsglineItems()
{
        return g_queue_get_length(&msgline);
}

/**
 * 查看消息队列首项.
 * @return 项
 */
GroupInfo *CoreThread::GetMsglineHeadItem()
{
        return (GroupInfo *)g_queue_peek_head(&msgline);
}

/**
 * 消息队列是否已经包含此项.
 * @param grpinf 项
 * @return 是否包含
 */
bool CoreThread::MsglineContainItem(GroupInfo *grpinf)
{
        return g_queue_find(&msgline, grpinf);
}

/**
 * 压入项进消息队列.
 * @param grpinf 项
 */
void CoreThread::PushItemToMsgline(GroupInfo *grpinf)
{
        g_queue_push_tail(&msgline, grpinf);
}

/**
 * 弹出项从消息队列.
 * @param grpinf 项
 */
void CoreThread::PopItemFromMsgline(GroupInfo *grpinf)
{
        g_queue_remove(&msgline, grpinf);
}

/**
 * 附加文件信息到公有文件链表.
 * @param file 文件信息
 */
void CoreThread::AttachFileToPublic(FileInfo *file)
{
        pblist = g_slist_append(pblist, file);
}

/**
 * 从公有文件链表删除指定的文件.
 * @param fileid 文件ID
 */
void CoreThread::DelFileFromPublic(uint32_t fileid)
{
        GSList *tlist;

        tlist = pblist;
        while (tlist) {
                if (((FileInfo *)tlist->data)->fileid == fileid) {
                        delete (FileInfo *)tlist->data;
                        pblist = g_slist_delete_link(pblist, tlist);
                        break;
                }
                tlist = g_slist_next(tlist);
        }
}

/**
 * 清空公有文件链表.
 */
void CoreThread::ClearFileFromPublic()
{
        for (GSList *tlist = pblist; tlist; tlist = g_slist_next(tlist))
                delete (FileInfo *)tlist->data;
        g_slist_free(pblist);
        pblist = NULL;
}

/**
 * 获取公有文件链表指针.
 * @return 链表
 */
GSList *CoreThread::GetPublicFileList()
{
        return pblist;
}

/**
 * 附加文件信息到私有文件链表.
 * @param file 文件信息
 */
void CoreThread::AttachFileToPrivate(FileInfo *file)
{
        prlist = g_slist_append(prlist, file);
}

/**
 * 从私有文件链表删除指定的文件.
 * @param fileid 文件ID
 */
void CoreThread::DelFileFromPrivate(uint32_t fileid)
{
        GSList *tlist;

        tlist = prlist;
        while (tlist) {
                if (((FileInfo *)tlist->data)->fileid == fileid) {
                        delete (FileInfo *)tlist->data;
                        prlist = g_slist_delete_link(prlist, tlist);
                        break;
                }
                tlist = g_slist_next(tlist);
        }
}

/**
 * 清空私有文件链表.
 */
void CoreThread::ClearFileFromPrivate()
{
        for (GSList *tlist = prlist; tlist; tlist = g_slist_next(tlist))
                delete (FileInfo *)tlist->data;
        g_slist_free(prlist);
        prlist = NULL;
}

/**
 * 获取指定文件ID的文件信息.
 * @param fileid 文件ID
 * @return 文件信息
 */
FileInfo *CoreThread::GetFileFromAll(uint32_t fileid)
{
        GSList *tlist;

        tlist = fileid < MAX_SHAREDFILE ? pblist : prlist;
        while (tlist) {
                if (((FileInfo *)tlist->data)->fileid == fileid)
                        break;
                tlist = g_slist_next(tlist);
        }

        return (FileInfo *)(tlist ? tlist->data : NULL);
}

/**
 * 获取指定文件包编号的文件信息.
 * @param PacketN 文件包ID
 * @return 文件信息
 * 这个函数主要是为了兼容adroid版的信鸽(IPMSG),IPMSG把包编号转换为
 * 16进制放在了本来应该是fileid的地方,在存放文件创建时间的地方放上了包内编号
 * 所以在调用这个函数时,传给packageNum的是fileid,
 * 传的filectime实际上是包内编号
 */
FileInfo *CoreThread::GetFileFromAllWithPacketN(uint32_t packageNum,uint32_t filectime)
{
    GSList *tlist;

    tlist = prlist;
    while (tlist) {
        if( (((FileInfo *)tlist->data)->packetn == packageNum)
                && ((((FileInfo *)tlist->data)->filenum == filectime) ) )
                    break;
            tlist = g_slist_next(tlist);
    }
    if (tlist != NULL)
            return (FileInfo *)(tlist ? tlist->data : NULL);
    tlist =  pblist ;
    while (tlist) {
         if( (((FileInfo *)tlist->data)->packetn == packageNum)
                && ((((FileInfo *)tlist->data)->filenum == filectime) ) )
                    break;
            tlist = g_slist_next(tlist);
    }
    return (FileInfo *)(tlist ? tlist->data : NULL);
}

/**
 * 获取共享文件访问密码.
 * @return 密码字符串
 */
const char *CoreThread::GetAccessPublicLimit()
{
        return passwd;
}

/**
 * 更新共享文件访问密码.
 * @param limit 密码字符串
 */
void CoreThread::SetAccessPublicLimit(const char *limit)
{
        g_free(passwd);
        passwd = g_strdup(limit);
}

/**
 * 初始化底层数据.
 */
void CoreThread::InitSublayer()
{
        InitThemeSublayerData();
        ReadSharedData();
}

/**
 * 清空底层数据.
 */
void CoreThread::ClearSublayer()
{
        GSList *tlist;

        /**
         * @note 必须在发送下线信息之后才能关闭套接口.
         */
        g_slist_foreach(pallist, GFunc(SendBroadcastExit), NULL);
        shutdown(tcpsock, SHUT_RDWR);
        shutdown(udpsock, SHUT_RDWR);
        server = false;

        for (tlist = pallist; tlist; tlist = g_slist_next(tlist))
                delete (PalInfo *)tlist->data;
        g_slist_free(pallist);
        for (tlist = rgllist; tlist; tlist = g_slist_next(tlist))
                delete (GroupInfo *)tlist->data;
        g_slist_free(rgllist);
        for (tlist = sgmlist; tlist; tlist = g_slist_next(tlist))
                delete (GroupInfo *)tlist->data;
        g_slist_free(sgmlist);
        for (tlist = grplist; tlist; tlist = g_slist_next(tlist))
                delete (GroupInfo *)tlist->data;
        g_slist_free(grplist);
        for (tlist = brdlist; tlist; tlist = g_slist_next(tlist))
                delete (GroupInfo *)tlist->data;
        g_slist_free(brdlist);
        g_slist_free(blacklist);
        g_queue_clear(&msgline);

        for (tlist = pblist; tlist; tlist = g_slist_next(tlist))
                delete (FileInfo *)tlist->data;
        g_slist_free(pblist);
        for (tlist = prlist; tlist; tlist = g_slist_next(tlist))
                delete (FileInfo *)tlist->data;
        g_slist_free(prlist);
        g_free(passwd);

        for (tlist = ecsList; tlist; tlist = g_slist_next(tlist))
                delete (FileInfo *)tlist->data;
        g_slist_free(ecsList);

        if (timerid > 0)
                g_source_remove(timerid);
        pthread_mutex_destroy(&mutex);
}

/**
 * 初始化主题库底层数据.
 */
void CoreThread::InitThemeSublayerData()
{
        GtkIconTheme *theme;
        GtkIconFactory *factory;
        GtkIconSet *set;
        GdkPixbuf *pixbuf;

        theme = gtk_icon_theme_get_default();
        gtk_icon_theme_append_search_path(theme, __PIXMAPS_PATH);
        gtk_icon_theme_append_search_path(theme, __PIXMAPS_PATH "/icon");
        gtk_icon_theme_append_search_path(theme, __PIXMAPS_PATH "/menu");
        gtk_icon_theme_append_search_path(theme, __PIXMAPS_PATH "/tip");

        factory = gtk_icon_factory_new();
        gtk_icon_factory_add_default(factory);
        if ( (pixbuf = gtk_icon_theme_load_icon(theme, "ip-tux", 64,
                                 GtkIconLookupFlags(0), NULL))) {
                set = gtk_icon_set_new_from_pixbuf(pixbuf);
                gtk_icon_factory_add(factory, "iptux-logo-show", set);
                g_object_unref(pixbuf);
        }
        if ( (pixbuf = gtk_icon_theme_load_icon(theme, "i-tux", 64,
                                 GtkIconLookupFlags(0), NULL))) {
                set = gtk_icon_set_new_from_pixbuf(pixbuf);
                gtk_icon_factory_add(factory, "iptux-logo-hide", set);
                g_object_unref(pixbuf);
        }
        g_object_unref(factory);
}

/**
 * 读取共享文件数据.
 */
void CoreThread::ReadSharedData()
{
        GConfClient *client;
        GSList *list, *tlist;
        FileInfo *file;
        struct stat64 st;

        /* 读取共享文件数据 */
        client = gconf_client_get_default();
        list = gconf_client_get_list(client, GCONF_PATH "/shared_file_list",
                                                 GCONF_VALUE_STRING, NULL);
        passwd = gconf_client_get_string(client, GCONF_PATH "/access_shared_limit", NULL);
        g_object_unref(client);

        /* 分析数据并加入文件链表 */
        for(tlist = list; tlist; tlist = g_slist_next(tlist)) {
                if (stat64((char *)tlist->data, &st) == -1
                         || !(S_ISREG(st.st_mode) || S_ISDIR(st.st_mode))) {
                        g_free(tlist->data);
                        tlist->data = NULL;
                        continue;
                }
                /* 加入文件信息到链表 */
                file = new FileInfo;
                pblist = g_slist_append(pblist, file);
                file->fileid = pbn++;
                /* file->packetn = 0;//没必要设置此字段 */
                file->fileattr = S_ISREG(st.st_mode) ? IPMSG_FILE_REGULAR :
                                                         IPMSG_FILE_DIR;
                /* file->filesize = 0;//我可不愿意程序启动时在这儿卡住 */
                /* file->fileown = NULL;//没必要设置此字段 */
                file->filepath = (char *)tlist->data;
        }
        g_slist_free(list);
}

/**
 * 插入消息头到TextBuffer(非UI线程安全).
 * @param buffer text-buffer
 * @param para 消息参数
 */
void CoreThread::InsertHeaderToBuffer(GtkTextBuffer *buffer, MsgPara *para)
{
        GtkTextIter iter;
        gchar *header;

        /**
         * @note (para->pal)可能为null.
         */
        switch (para->stype) {
        case MESSAGE_SOURCE_TYPE_PAL:
                header = getformattime(FALSE, "%s", para->pal ? para->pal->name : _("unknown"));
                gtk_text_buffer_get_end_iter(buffer, &iter);
                gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                                                         header, -1, "pal-color", NULL);
                g_free(header);
                break;
        case MESSAGE_SOURCE_TYPE_SELF:
                header = getformattime(FALSE, "%s", progdt.nickname);
                gtk_text_buffer_get_end_iter(buffer, &iter);
                gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                                         header, -1, "me-color", NULL);
                g_free(header);
                break;
        case MESSAGE_SOURCE_TYPE_ERROR:
                header = getformattime(FALSE, "%s", _("<ERROR>"));
                gtk_text_buffer_get_end_iter(buffer, &iter);
                gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                                                         header, -1, "error-color", NULL);
                g_free(header);
                break;
        default:
                break;
        }
}

/**
 * 插入字符串到TextBuffer(非UI线程安全).
 * @param buffer text-buffer
 * @param string 字符串
 */
void CoreThread::InsertStringToBuffer(GtkTextBuffer *buffer, gchar *string)
{
        static uint32_t count = 0;
        GtkTextIter iter;
        GtkTextTag *tag;
        GMatchInfo *matchinfo;
        gchar *substring;
        char name[9];   //8 +1  =9
        gint startp, endp;
        gint urlendp;

        urlendp = 0;
        matchinfo = NULL;
        gtk_text_buffer_get_end_iter(buffer, &iter);
        g_regex_match_full(progdt.urlregex, string, -1, 0, GRegexMatchFlags(0),
                                                         &matchinfo, NULL);
        while (g_match_info_matches(matchinfo))
        {
                snprintf(name, 9, "%" PRIx32, count++);
                tag = gtk_text_buffer_create_tag(buffer, name, NULL);
                substring = g_match_info_fetch(matchinfo, 0);
                g_object_set_data_full(G_OBJECT(tag), "url",  substring,
                                                 GDestroyNotify(g_free));
                g_match_info_fetch_pos(matchinfo, 0, &startp, &endp);
                gtk_text_buffer_insert(buffer, &iter, string + urlendp, startp - urlendp);
                gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, string + startp,
                                                 endp - startp, "url-link", name, NULL);
                urlendp = endp;
                g_match_info_next(matchinfo, NULL);
        }
        g_match_info_free(matchinfo);
        gtk_text_buffer_insert(buffer, &iter, string + urlendp, -1);
}

/**
 * 插入图片到TextBuffer(非UI线程安全).
 * @param buffer text-buffer
 * @param path 图片路径
 */
void CoreThread::InsertPixbufToBuffer(GtkTextBuffer *buffer, gchar *path)
{
        GtkTextIter start, end;
        GdkPixbuf *pixbuf;

        if ( (pixbuf = gdk_pixbuf_new_from_file(path, NULL))) {
                gtk_text_buffer_get_start_iter(buffer, &start);
                if (gtk_text_iter_get_char(&start) == OCCUPY_OBJECT
                         || gtk_text_iter_forward_find_char(&start,
                                 GtkTextCharPredicate(giter_compare_foreach),
                                 GUINT_TO_POINTER(OCCUPY_OBJECT), NULL)) {
                        end = start;
                        gtk_text_iter_forward_char(&end);
                        gtk_text_buffer_delete(buffer, &start, &end);
                }
                gtk_text_buffer_insert_pixbuf(buffer, &start, pixbuf);
                g_object_unref(pixbuf);
        }
}

/**
 * 获取(pal)在分组模式下当前所在的群组.
 * @param pal class PalInfo
 * @return 群组信息
 */
GroupInfo *CoreThread::GetPalPrevGroupItem(PalInfo *pal)
{
        GSList *tlist;

        tlist = grplist;
        while (tlist) {
                if (g_slist_find(((GroupInfo *)tlist->data)->member, pal))
                        break;
                tlist = g_slist_next(tlist);
        }

        return (GroupInfo *)(tlist ? tlist->data : NULL);
}

/**
 * 增加新项到常规模式群组链表(非UI线程安全).
 * @param pal class PalInfo
 * @return 新加入的群组
 */
GroupInfo *CoreThread::AttachPalRegularItem(PalInfo *pal)
{
        GroupInfo *grpinf;

        grpinf = new GroupInfo;
        grpinf->grpid = pal->ipv4;
        grpinf->type = GROUP_BELONG_TYPE_REGULAR;
        grpinf->name = g_strdup(pal->name);
        grpinf->member = NULL;
        grpinf->buffer = gtk_text_buffer_new(progdt.table);
        grpinf->dialog = NULL;
        rgllist = g_slist_append(rgllist, grpinf);

        return grpinf;
}

/**
 * 增加新项到网段模式群组链表(非UI线程安全).
 * @param pal class PalInfo
 * @return 新加入的群组
 */
GroupInfo *CoreThread::AttachPalSegmentItem(PalInfo *pal)
{
        GroupInfo *grpinf;
        char *name;

        /* 获取局域网网段名称 */
        name = ipv4_get_lan_name(pal->ipv4);
        name = name ? name : g_strdup(_("Others"));

        grpinf = new GroupInfo;
        grpinf->grpid = g_quark_from_static_string(name);
        grpinf->type = GROUP_BELONG_TYPE_SEGMENT;
        grpinf->name = name;
        grpinf->member = NULL;
        grpinf->buffer = gtk_text_buffer_new(progdt.table);
        grpinf->dialog = NULL;
        sgmlist = g_slist_append(sgmlist, grpinf);

        return grpinf;
}

/**
 * 增加新项到分组模式群组链表(非UI线程安全).
 * @param pal class PalInfo
 * @return 新加入的群组
 */
GroupInfo *CoreThread::AttachPalGroupItem(PalInfo *pal)
{
        GroupInfo *grpinf;
        char *name;

        /* 备份组名称，用于计算ID号 */
        NO_OPERATION_C
        name = g_strdup(pal->group ? pal->group : _("Others"));

        grpinf = new GroupInfo;
        grpinf->grpid = g_quark_from_static_string(name);
        grpinf->type = GROUP_BELONG_TYPE_GROUP;
        grpinf->name = name;
        grpinf->member = NULL;
        grpinf->buffer = gtk_text_buffer_new(progdt.table);
        grpinf->dialog = NULL;
        grplist = g_slist_append(grplist, grpinf);

        return grpinf;
}

/**
 * 增加新项到广播模式群组链表(非UI线程安全).
 * @param pal class PalInfo
 * @return 新加入的群组
 */
GroupInfo *CoreThread::AttachPalBroadcastItem(PalInfo *pal)
{
        GroupInfo *grpinf;
        char *name;

        name = g_strdup(_("Broadcast"));

        grpinf = new GroupInfo;
        grpinf->grpid = g_quark_from_static_string(name);
        grpinf->type = GROUP_BELONG_TYPE_BROADCAST;
        grpinf->name = name;
        grpinf->member = NULL;
        grpinf->buffer = gtk_text_buffer_new(progdt.table);
        grpinf->dialog = NULL;
        brdlist = g_slist_append(brdlist, grpinf);

        return grpinf;
}

/**
 * 从群组中移除指定的好友(非UI线程安全).
 * @param grpinf class GroupInfo
 * @param pal class PalInfo
 */
void CoreThread::DelPalFromGroupInfoItem(GroupInfo *grpinf, PalInfo *pal)
{
        GSList *tlist;
        SessionAbstract *session;

        if ( (tlist = g_slist_find(grpinf->member, pal))) {
                grpinf->member = g_slist_delete_link(grpinf->member, tlist);
                if (grpinf->dialog) {
                        session = (SessionAbstract *)g_object_get_data(G_OBJECT(
                                                 grpinf->dialog), "session-class");
                        session->DelPalData(pal);
                }
        }
}

/**
 * 添加好友到指定的群组(非UI线程安全).
 * @param grpinf class GroupInfo
 * @param pal class PalInfo
 */
void CoreThread::AttachPalToGroupInfoItem(GroupInfo *grpinf, PalInfo *pal)
{
        SessionAbstract *session;

        grpinf->member = g_slist_append(grpinf->member, pal);
        if (grpinf->dialog) {
                session = (SessionAbstract *)g_object_get_data(G_OBJECT(
                                         grpinf->dialog), "session-class");
                session->InsertPalData(pal);
        }
}

/**
 * 监听UDP服务端口.
 * @param pcthrd 核心类
 */
void CoreThread::RecvUdpData(CoreThread *pcthrd)
{
        struct sockaddr_in addr;
        socklen_t len;
        char buf[MAX_UDPLEN];
        ssize_t size;

        while (pcthrd->server) {
                len = sizeof(addr);
                if ((size = recvfrom(pcthrd->udpsock, buf, MAX_UDPLEN, 0,
                                 (struct sockaddr *)&addr, &len)) == -1)
                        continue;
                if (size != MAX_UDPLEN)
                        buf[size] = '\0';
                UdpData::UdpDataEntry(addr.sin_addr.s_addr, buf, size);
        }
}

/**
 * 监听TCP服务端口.
 * @param pcthrd 核心类
 */
void CoreThread::RecvTcpData(CoreThread *pcthrd)
{
        pthread_t pid;
        int subsock;

        listen(pcthrd->tcpsock, 5);
        while (pcthrd->server) {
                if ((subsock = accept(pcthrd->tcpsock, NULL, NULL)) == -1)
                        continue;
                pthread_create(&pid, NULL, ThreadFunc(TcpData::TcpDataEntry),
                                                 GINT_TO_POINTER(subsock));
                pthread_detach(pid);
        }
}

/**
 * 扫描处理程序内部任务(非UI线程安全).
 * @param pcthrd 核心类
 * @return GLib库所需
 */
gboolean CoreThread::WatchCoreStatus(CoreThread *pcthrd)
{
        GList *tlist;

        /* 让等待队列中的群组信息项闪烁 */
        pthread_mutex_lock(&pcthrd->mutex);
        tlist = pcthrd->msgline.head;
        while (tlist) {
                mwin.MakeItemBlinking((GroupInfo *)tlist->data, true);
                tlist = g_list_next(tlist);
        }
        pthread_mutex_unlock(&pcthrd->mutex);

        return TRUE;
}
/**
 * 获取特定好友发过来的文件(非UI线程安全).
 * @param pal class PalInfo
 * @return palecslist 该好友发过来待接收的文件列表
 */
GSList *CoreThread::GetPalEnclosure(PalInfo *pal)
{
    GSList *tlist,*palecslist;
    palecslist = NULL;
    for (tlist = ecsList; tlist; tlist = g_slist_next(tlist)) {
        if (((FileInfo *)tlist->data)->fileown == pal) {
            palecslist = g_slist_append(palecslist,tlist->data);
        }
    }
    return palecslist;
}
/**
 * 压入项进接收文件列表(非UI线程安全).
 * @param file 文件类指针
 */
void CoreThread::PushItemToEnclosureList(FileInfo *file)
{
    ecsList = g_slist_append(ecsList, file);
}
/**
 * 从接收文件列表删除项(非UI线程安全).
 * @param file 文件类指针
 */
void CoreThread::PopItemFromEnclosureList(FileInfo *file)
{
    ecsList = g_slist_remove(ecsList, file);
    delete file;
}
