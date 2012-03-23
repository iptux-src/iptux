//
// C++ Implementation: UdpData
//
// Description:
//
//
// Author: cwll <cwll2009@126.com> ,(C)2012
//        Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
// 2012.02 在接收消息中去掉了群组模式，群组模式只用来发消息
//
#include "UdpData.h"
#include "ProgramData.h"
#include "CoreThread.h"
#include "MainWindow.h"
#include "DialogPeer.h"
#include "DialogGroup.h"
#include "SoundSystem.h"
#include "Command.h"
#include "RecvFile.h"
#include "SendFile.h"
#include "wrapper.h"
#include "dialog.h"
#include "utils.h"
extern ProgramData progdt;
extern CoreThread cthrd;
extern MainWindow mwin;
extern SoundSystem sndsys;

/**
 * 类构造函数.
 */
UdpData::UdpData():ipv4(0), size(0), encode(NULL)
{
}

/**
 * 类析构函数.
 */
UdpData::~UdpData()
{
        g_free(encode);
}

/**
 * UDP数据解析入口.
 * @param ipv4 ipv4
 * @param buf[] 数据缓冲区
 * @param size 数据有效长度
 */
void UdpData::UdpDataEntry(in_addr_t ipv4, const char buf[], size_t size)
{
        UdpData udata;

        udata.ipv4 = ipv4;
        udata.size = size < MAX_UDPLEN ? size : MAX_UDPLEN;
        memcpy(udata.buf, buf, size);
        if (size != MAX_UDPLEN)
                udata.buf[size] = '\0';
        udata.DispatchUdpData();
}

/**
 * 分派UDP数据到相应的函数去进行处理.
 */
void UdpData::DispatchUdpData()
{
        uint32_t commandno;

        /* 如果开启了黑名单处理功能，且此地址正好被列入了黑名单 */
        /* 嘿嘿，那就不要怪偶心狠手辣了 */
        if (FLAG_ISSET(progdt.flags, 1) && cthrd.BlacklistContainItem(ipv4))
                return;

        /* 决定消息去向 */
        commandno = iptux_get_dec_number(buf, ':', 4);
        switch (GET_MODE(commandno)) {
        case IPMSG_BR_ENTRY:
                SomeoneEntry();
                break;
        case IPMSG_BR_EXIT:
                SomeoneExit();
                break;
        case IPMSG_ANSENTRY:
                SomeoneAnsentry();
                break;
        case IPMSG_BR_ABSENCE:
                SomeoneAbsence();
                break;
        case IPMSG_SENDMSG:
                SomeoneSendmsg();
                break;
        case IPMSG_RECVMSG:
                SomeoneRecvmsg();
                break;
        case IPTUX_ASKSHARED:
                SomeoneAskShared();
                break;
        case IPTUX_SENDICON:
                SomeoneSendIcon();
                break;
        case IPTUX_SENDSIGN:
                SomeoneSendSign();
                break;
        case IPTUX_SENDMSG:
                SomeoneBcstmsg();
                break;
        default:
                break;
        }
}

/**
 * 好友信息数据丢失默认处理函数.
 * 若xx并不在好友列表中，但是程序却期望能够接受xx的本次会话，
 * 所以必须以默认数据构建好友信息数据并插入好友列表 \n
 */
void UdpData::SomeoneLost()
{
        PalInfo *pal;

        /* 创建好友数据 */
        pal = new PalInfo;
        pal->ipv4 = ipv4;
        pal->segdes = progdt.FindNetSegDescription(ipv4);
        if (!(pal->version = iptux_get_section_string(buf, ':', 0)))
                pal->version = g_strdup("?");
        if (!(pal->user = iptux_get_section_string(buf, ':', 2)))
                pal->user = g_strdup("???");
        if (!(pal->host = iptux_get_section_string(buf, ':', 3)))
                pal->host = g_strdup("???");
        pal->name = g_strdup(_("mysterious"));
        pal->group = g_strdup(_("mysterious"));
        pal->photo = NULL;
        pal->sign = NULL;
        pal->iconfile = g_strdup(progdt.palicon);
        pal->encode = g_strdup(encode ? encode : "utf-8");
        FLAG_SET(pal->flags, 1);
        pal->packetn = 0;
        pal->rpacketn = 0;

        /* 加入好友列表 */
        gdk_threads_enter();
        pthread_mutex_lock(cthrd.GetMutex());
        cthrd.AttachPalToList(pal);
        pthread_mutex_unlock(cthrd.GetMutex());
        mwin.AttachItemToPaltree(ipv4);
        gdk_threads_leave();
}

/**
 * 好友上线.
 */
void UdpData::SomeoneEntry()
{
        Command cmd;
        pthread_t pid;
        PalInfo *pal;

        /* 转换缓冲区数据编码 */
        ConvertEncode(progdt.encode);

        /* 加入或更新好友列表 */
        gdk_threads_enter();
        pthread_mutex_lock(cthrd.GetMutex());
        if ( (pal = cthrd.GetPalFromList(ipv4))) {
                UpdatePalInfo(pal);
                cthrd.UpdatePalToList(ipv4);
        } else {
                pal = CreatePalInfo();
                cthrd.AttachPalToList(pal);
        }
        pthread_mutex_unlock(cthrd.GetMutex());
        if (mwin.PaltreeContainItem(ipv4))
                mwin.UpdateItemToPaltree(ipv4);
        else
                mwin.AttachItemToPaltree(ipv4);
        gdk_threads_leave();

        /* 通知好友本大爷在线 */
        cmd.SendAnsentry(cthrd.UdpSockQuote(), pal);
        if (FLAG_ISSET(pal->flags, 0)) {
                pthread_create(&pid, NULL, ThreadFunc(CoreThread::SendFeatureData), pal);
                pthread_detach(pid);
        }
}

/**
 * 好友退出.
 */
void UdpData::SomeoneExit()
{
        PalInfo *pal;

        /* 从好友链表中删除 */
        gdk_threads_enter();
        if (mwin.PaltreeContainItem(ipv4))
                mwin.DelItemFromPaltree(ipv4);
        pthread_mutex_lock(cthrd.GetMutex());
        if ( (pal = cthrd.GetPalFromList(ipv4))) {
                cthrd.DelPalFromList(ipv4);
                FLAG_CLR(pal->flags, 1);
        }
        pthread_mutex_unlock(cthrd.GetMutex());
        gdk_threads_leave();
}

/**
 * 好友在线.
 */
void UdpData::SomeoneAnsentry()
{
        Command cmd;
        pthread_t pid;
        PalInfo *pal;
        const char *ptr;

        /* 若好友不兼容iptux协议，则需转码 */
        ptr = iptux_skip_string(buf, size, 3);
        if (!ptr || *ptr == '\0')
                ConvertEncode(progdt.encode);

        /* 加入或更新好友列表 */
        gdk_threads_enter();
        pthread_mutex_lock(cthrd.GetMutex());
        if ( (pal = cthrd.GetPalFromList(ipv4))) {
                UpdatePalInfo(pal);
                cthrd.UpdatePalToList(ipv4);
        } else {
                pal = CreatePalInfo();
                cthrd.AttachPalToList(pal);
        }
        pthread_mutex_unlock(cthrd.GetMutex());
        if (mwin.PaltreeContainItem(ipv4))
                mwin.UpdateItemToPaltree(ipv4);
        else
                mwin.AttachItemToPaltree(ipv4);
        gdk_threads_leave();

        /* 更新本大爷的数据信息 */
        if (FLAG_ISSET(pal->flags, 0)) {
                pthread_create(&pid, NULL, ThreadFunc(CoreThread::SendFeatureData), pal);
                pthread_detach(pid);
        } else if (strcasecmp(progdt.encode, pal->encode) != 0)
                cmd.SendAnsentry(cthrd.UdpSockQuote(), pal);
}

/**
 * 好友更改个人信息.
 */
void UdpData::SomeoneAbsence()
{
        PalInfo *pal;
        const char *ptr;

        /* 若好友不兼容iptux协议，则需转码 */
        pal = cthrd.GetPalFromList(ipv4);       //利用好友链表只增不减的特性，无须加锁
        ptr = iptux_skip_string(buf, size, 3);
        if (!ptr || *ptr == '\0')
                ConvertEncode(pal ? pal->encode : progdt.encode);

        /* 加入或更新好友列表 */
        gdk_threads_enter();
        pthread_mutex_lock(cthrd.GetMutex());
        if (pal) {
                UpdatePalInfo(pal);
                cthrd.UpdatePalToList(ipv4);
        } else {
                pal = CreatePalInfo();
                cthrd.AttachPalToList(pal);
        }
        pthread_mutex_unlock(cthrd.GetMutex());
        if (mwin.PaltreeContainItem(ipv4))
                mwin.UpdateItemToPaltree(ipv4);
        else
                mwin.AttachItemToPaltree(ipv4);
        gdk_threads_leave();
}

/**
 * 好友发送消息.
 *
 */
void UdpData::SomeoneSendmsg()
{
        GroupInfo *grpinf;
        PalInfo *pal;
        Command cmd;
        uint32_t commandno, packetno;
        char *text;
        pthread_t pid;
        DialogPeer *dlgpr;
        GtkWidget *window;

        /* 如果对方兼容iptux协议，则无须再转换编码 */
        pal = cthrd.GetPalFromList(ipv4);
        if (!pal || !FLAG_ISSET(pal->flags, 0))
                ConvertEncode(pal ? pal->encode : progdt.encode);
        /* 确保好友在线，并对编码作出适当调整 */
        pal = AssertPalOnline();
        if (strcasecmp(pal->encode, encode ? encode : "utf-8") != 0) {
                g_free(pal->encode);
                pal->encode = g_strdup(encode ? encode : "utf-8");
        }

        /* 回复好友并检查此消息是否过时 */
        commandno = iptux_get_dec_number(buf, ':', 4);
        packetno = iptux_get_dec_number(buf, ':', 1);
        if (commandno & IPMSG_SENDCHECKOPT)
                cmd.SendReply(cthrd.UdpSockQuote(), pal, packetno);
        if (packetno <= pal->packetn)
                return;
        pal->packetn = packetno;

        /* 插入消息&在消息队列中注册 */
        text = ipmsg_get_attach(buf, ':', 5);
        if (text && *text != '\0') {
                /*/* 插入消息 */
//                if ((commandno & IPMSG_BROADCASTOPT) || (commandno & IPMSG_MULTICASTOPT))
//                        InsertMessage(pal, GROUP_BELONG_TYPE_BROADCAST, text);
//                else
                        InsertMessage(pal, GROUP_BELONG_TYPE_REGULAR, text);
        }
        g_free(text);
        /*/* 注册消息 */
        pthread_mutex_lock(cthrd.GetMutex());
//        if ((commandno & IPMSG_BROADCASTOPT) || (commandno & IPMSG_MULTICASTOPT))
//                grpinf = cthrd.GetPalBroadcastItem(pal);
//        else
                grpinf = cthrd.GetPalRegularItem(pal);
        if (!grpinf->dialog && !cthrd.MsglineContainItem(grpinf))
                cthrd.PushItemToMsgline(grpinf);
        pthread_mutex_unlock(cthrd.GetMutex());

        /* 标记位处理 先处理底层数据，后面显示窗口*/
        if (commandno & IPMSG_FILEATTACHOPT) {
                if ((commandno & IPTUX_SHAREDOPT) && (commandno & IPTUX_PASSWDOPT)) {
                        pthread_create(&pid, NULL, ThreadFunc(ThreadAskSharedPasswd), pal);
                        pthread_detach(pid);
                } else
                        RecvPalFile();
        }
        window = GTK_WIDGET(grpinf->dialog);
        //这里不知道为什么运行时一直会提示window不是object
        dlgpr = (DialogPeer *)(g_object_get_data(G_OBJECT(window),"dialog"));
        if(grpinf->dialog)
            dlgpr->ShowDialogPeer(dlgpr);
        /* 是否直接弹出聊天窗口 */
        if (FLAG_ISSET(progdt.flags, 7)) {
                gdk_threads_enter();
                if (!(grpinf->dialog)) {
//                     switch (grpinf->type) {
//                     case GROUP_BELONG_TYPE_REGULAR:
                          DialogPeer::PeerDialogEntry(grpinf);
//                          break;
//                     case GROUP_BELONG_TYPE_SEGMENT:
//                     case GROUP_BELONG_TYPE_GROUP:
//                     case GROUP_BELONG_TYPE_BROADCAST:
//                          DialogGroup::GroupDialogEntry(grpinf);
//                     default:
//                          break;
//                     }
                } else {
                     gtk_window_present(GTK_WINDOW(grpinf->dialog));
                }
                gdk_threads_leave();
        }

        /* 播放提示音 */
        if (FLAG_ISSET(progdt.sndfgs, 1))
                sndsys.Playing(progdt.msgtip);
}

/**
 * 好友接收到消息.
 */
void UdpData::SomeoneRecvmsg()
{
        uint32_t packetno;
        PalInfo *pal;

        if ( (pal = cthrd.GetPalFromList(ipv4))) {
                packetno = iptux_get_dec_number(buf, ':', 5);
                if (packetno == pal->rpacketn)
                        pal->rpacketn = 0;      //标记此包编号已经被回复
        }
}

/**
 * 好友请求本计算机的共享文件.
 */
void UdpData::SomeoneAskShared()
{
        Command cmd;
        pthread_t pid;
        PalInfo *pal;
        const char *limit;
        char *passwd;

        if (!(pal = cthrd.GetPalFromList(ipv4)))
                return;

        limit = cthrd.GetAccessPublicLimit();
        if (!limit || *limit == '\0') {
                pthread_create(&pid, NULL, ThreadFunc(ThreadAskSharedFile), pal);
                pthread_detach(pid);
        } else if (!(iptux_get_dec_number(buf, ':', 4) & IPTUX_PASSWDOPT)) {
                cmd.SendFileInfo(cthrd.UdpSockQuote(), pal,
                         IPTUX_SHAREDOPT | IPTUX_PASSWDOPT, "");
        } else if ( (passwd = ipmsg_get_attach(buf, ':', 5))) {
                if (strcmp(passwd, limit) == 0) {
                        pthread_create(&pid, NULL, ThreadFunc(ThreadAskSharedFile), pal);
                        pthread_detach(pid);
                }
                g_free(passwd);
        }
}

/**
 * 好友发送头像数据.
 */
void UdpData::SomeoneSendIcon()
{
        PalInfo *pal;
        char *iconfile;

        if (!(pal = cthrd.GetPalFromList(ipv4)) || FLAG_ISSET(pal->flags, 2))
                return;

        /* 接收并更新数据 */
        if ( (iconfile = RecvPalIcon())) {
                g_free(pal->iconfile);
                pal->iconfile = iconfile;
                gdk_threads_enter();
                pthread_mutex_lock(cthrd.GetMutex());
                cthrd.UpdatePalToList(ipv4);
                pthread_mutex_unlock(cthrd.GetMutex());
                mwin.UpdateItemToPaltree(ipv4);
                gdk_threads_leave();
        }
}

/**
 * 好友发送个性签名.
 */
void UdpData::SomeoneSendSign()
{
        PalInfo *pal;
        char *sign;

        if (!(pal = cthrd.GetPalFromList(ipv4)))
                return;

        /* 若好友不兼容iptux协议，则需转码 */
        if (!FLAG_ISSET(pal->flags, 0))
                ConvertEncode(pal->encode);
        /* 对编码作适当调整 */
        if (strcasecmp(pal->encode, encode ? encode : "utf-8") != 0) {
                g_free(pal->encode);
                pal->encode = g_strdup(encode ? encode : "utf-8");
        }
        /* 更新 */
        if ( (sign = ipmsg_get_attach(buf, ':', 5))) {
                g_free(pal->sign);
                pal->sign = sign;
                gdk_threads_enter();
                pthread_mutex_lock(cthrd.GetMutex());
                cthrd.UpdatePalToList(ipv4);
                pthread_mutex_unlock(cthrd.GetMutex());
                mwin.UpdateItemToPaltree(ipv4);
                gdk_threads_leave();
        }
}

/**
 * 好友广播消息.
 */
void UdpData::SomeoneBcstmsg()
{
        GroupInfo *grpinf;
        PalInfo *pal;
        uint32_t commandno, packetno;
        char *text;

        /* 如果对方兼容iptux协议，则无须再转换编码 */
        pal = cthrd.GetPalFromList(ipv4);
        if (!pal || !FLAG_ISSET(pal->flags, 0))
                ConvertEncode(pal ? pal->encode : progdt.encode);
        /* 确保好友在线，并对编码作出适当调整 */
        pal = AssertPalOnline();
        if (strcasecmp(pal->encode, encode ? encode : "utf-8") != 0) {
                g_free(pal->encode);
                pal->encode = g_strdup(encode ? encode : "utf-8");
        }

        /* 检查此消息是否过时 */
        packetno = iptux_get_dec_number(buf, ':', 1);
        if (packetno <= pal->packetn)
                return;
        pal->packetn = packetno;

        /* 插入消息&在消息队列中注册 */
        text = ipmsg_get_attach(buf, ':', 5);
        if (text && *text != '\0') {
                commandno = iptux_get_dec_number(buf, ':', 4);
                /*/* 插入消息 */
                switch (GET_OPT(commandno)) {
                case IPTUX_BROADCASTOPT:
                        InsertMessage(pal, GROUP_BELONG_TYPE_BROADCAST, text);
                        break;
                case IPTUX_GROUPOPT:
                        InsertMessage(pal, GROUP_BELONG_TYPE_GROUP, text);
                        break;
                case IPTUX_SEGMENTOPT:
                        InsertMessage(pal, GROUP_BELONG_TYPE_SEGMENT, text);
                        break;
                case IPTUX_REGULAROPT:
                default:
                        InsertMessage(pal, GROUP_BELONG_TYPE_REGULAR, text);
                        break;
                }
                /*/* 注册消息 */
                pthread_mutex_lock(cthrd.GetMutex());
                switch (GET_OPT(commandno)) {
                case IPTUX_BROADCASTOPT:
                        grpinf = cthrd.GetPalBroadcastItem(pal);
                        break;
                case IPTUX_GROUPOPT:
                        grpinf = cthrd.GetPalGroupItem(pal);
                        break;
                case IPTUX_SEGMENTOPT:
                        grpinf = cthrd.GetPalSegmentItem(pal);
                        break;
                case IPTUX_REGULAROPT:
                default:
                        grpinf = cthrd.GetPalRegularItem(pal);
                        break;
                }
                if (!grpinf->dialog && !cthrd.MsglineContainItem(grpinf))
                        cthrd.PushItemToMsgline(grpinf);
                pthread_mutex_unlock(cthrd.GetMutex());
        }
        g_free(text);

        /* 播放提示音 */
        if (FLAG_ISSET(progdt.sndfgs, 1))
                sndsys.Playing(progdt.msgtip);
}

/**
 * 创建好友信息数据.
 * @return 好友数据
 */
PalInfo *UdpData::CreatePalInfo()
{
        PalInfo *pal;

        pal = new PalInfo;
        pal->ipv4 = ipv4;
        pal->segdes = progdt.FindNetSegDescription(ipv4);
        if (!(pal->version = iptux_get_section_string(buf, ':', 0)))
                pal->version = g_strdup("?");
        if (!(pal->user = iptux_get_section_string(buf, ':', 2)))
                pal->user = g_strdup("???");
        if (!(pal->host = iptux_get_section_string(buf, ':', 3)))
                pal->host = g_strdup("???");
        if (!(pal->name = ipmsg_get_attach(buf, ':', 5)))
                pal->name = g_strdup(_("mysterious"));
        pal->group = GetPalGroup();
        pal->photo = NULL;
        pal->sign = NULL;
        if (!(pal->iconfile = GetPalIcon()))
                pal->iconfile = g_strdup(progdt.palicon);
        if ( (pal->encode = GetPalEncode()))
                FLAG_SET(pal->flags, 0);
        else
                pal->encode = g_strdup(encode ? encode : "utf-8");
        FLAG_SET(pal->flags, 1);
        pal->packetn = 0;
        pal->rpacketn = 0;

        return pal;
}

/**
 * 更新好友信息数据.
 * @param pal 好友数据
 */
void UdpData::UpdatePalInfo(PalInfo *pal)
{
        g_free(pal->segdes);
        pal->segdes = progdt.FindNetSegDescription(ipv4);
        g_free(pal->version);
        if (!(pal->version = iptux_get_section_string(buf, ':', 0)))
                pal->version = g_strdup("?");
        g_free(pal->user);
        if (!(pal->user = iptux_get_section_string(buf, ':', 2)))
                pal->user = g_strdup("???");
        g_free(pal->host);
        if (!(pal->host = iptux_get_section_string(buf, ':', 3)))
                pal->host = g_strdup("???");
        if (!FLAG_ISSET(pal->flags, 2)) {
                g_free(pal->name);
                if (!(pal->name = ipmsg_get_attach(buf, ':', 5)))
                        pal->name = g_strdup(_("mysterious"));
                g_free(pal->group);
                pal->group = GetPalGroup();
                g_free(pal->iconfile);
                if (!(pal->iconfile = GetPalIcon()))
                        pal->iconfile = g_strdup(progdt.palicon);
                FLAG_CLR(pal->flags, 0);
                g_free(pal->encode);
                if ( (pal->encode = GetPalEncode()))
                        FLAG_SET(pal->flags, 0);
                else
                        pal->encode = g_strdup(encode ? encode : "utf-8");
        }
        FLAG_SET(pal->flags, 1);
        pal->packetn = 0;
        pal->rpacketn = 0;
}

/**
 * 插入消息.
 * @param pal class PalInfo
 * @param btype 消息所属类型
 * @param msg 消息
 */
void UdpData::InsertMessage(PalInfo *pal, GroupBelongType btype, const char *msg)
{
        MsgPara para;
        ChipData *chip;

        /* 构建消息封装包 */
        para.pal = pal;
        para.stype = MESSAGE_SOURCE_TYPE_PAL;
        para.btype = btype;
        chip = new ChipData;
        chip->type = MESSAGE_CONTENT_TYPE_STRING;
        chip->data = g_strdup(msg);
        para.dtlist = g_slist_append(NULL, chip);

        /* 交给某人处理吧 */
        cthrd.InsertMessage(&para);
}

/**
 * 将缓冲区中的数据转换为utf8编码.
 * @param enc 原数据首选编码
 */
void UdpData::ConvertEncode(const char *enc)
{
        char *ptr;
        size_t len;

        /* 将缓冲区内有效的'\0'字符转换为ASCII字符 */
        ptr = buf + strlen(buf) + 1;
        while ((size_t)(ptr - buf) <= size) {
                *(ptr - 1) = NULL_OBJECT;
                ptr += strlen(ptr) + 1;
        }

        /* 转换字符集编码 */
        /**
         * @note 请不要采用以下做法，它在某些环境下将导致致命错误: \n
         * if (g_utf8_validate(buf, -1, NULL)) {encode = g_strdup("utf-8")} \n
         * e.g. 系统编码为GB18030的xx发送来纯ASCII字符串 \n
         */
        if (enc && strcasecmp(enc, "utf-8") != 0
                 && (ptr = convert_encode(buf, "utf-8", enc)))
                encode = g_strdup(enc);
        else
                ptr = iptux_string_validate(buf, progdt.codeset, &encode);
        if (ptr) {
                len = strlen(ptr);
                size = len < MAX_UDPLEN ? len : MAX_UDPLEN;
                memcpy(buf, ptr, size);
                if (size < MAX_UDPLEN)
                        buf[size] = '\0';
                g_free(ptr);
        }

        /* 将缓冲区内的ASCII字符还原为'\0'字符 */
        ptr = buf;
        while ( (ptr = (char *)memchr(ptr, NULL_OBJECT, buf + size - ptr))) {
                *ptr = '\0';
                ptr++;
        }
}

/**
 * 获取好友群组名称.
 * @return 群组
 */
char *UdpData::GetPalGroup()
{
        const char *ptr;

        if ((ptr = iptux_skip_string(buf, size, 1)) && *ptr != '\0')
                return g_strdup(ptr);
        return NULL;
}

/**
 * 获取好友头像图标.
 * @return 头像
 */
char *UdpData::GetPalIcon()
{
        char path[MAX_PATHLEN];
        const char *ptr;

        if ((ptr = iptux_skip_string(buf, size, 2)) && *ptr != '\0') {
                snprintf(path, MAX_PATHLEN, __PIXMAPS_PATH "/icon/%s", ptr);
                if (access(path, F_OK) == 0)
                        return g_strdup(ptr);
        }
        return NULL;
}

/**
 * 获取好友系统编码.
 * @return 编码
 */
char *UdpData::GetPalEncode()
{
        const char *ptr;

        if ((ptr = iptux_skip_string(buf, size, 3)) && *ptr != '\0')
                return g_strdup(ptr);
        return NULL;
}

/**
 * 接收好友头像数据.
 * @return 头像文件名
 */
char *UdpData::RecvPalIcon()
{
        GdkPixbuf *pixbuf;
        char path[MAX_PATHLEN];
        char *iconfile;
        size_t len;
        int fd;

        /* 若无头像数据则返回null */
        if ((len = strlen(buf) + 1) >= size)
                return NULL;

        /* 将头像数据刷入磁盘 */
        snprintf(path, MAX_PATHLEN, "%s" ICON_PATH "/%" PRIx32,
                                 g_get_user_cache_dir(), ipv4);
        if ((fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
                return NULL;
        xwrite(fd, buf + len, size - len);
        close(fd);

        /* 将头像pixbuf加入内建 */
        iconfile = NULL;
        gdk_threads_enter();
        if ( (pixbuf = gdk_pixbuf_new_from_file(path, NULL))) {
                iconfile = g_strdup_printf("%" PRIx32, ipv4);
                gtk_icon_theme_add_builtin_icon(iconfile, MAX_ICONSIZE, pixbuf);
                g_object_unref(pixbuf);
        }
        gdk_threads_leave();

        return iconfile;
}

/**
 * 确保好友一定在线.
 * @return 好友数据
 */
PalInfo *UdpData::AssertPalOnline()
{
        PalInfo *pal;

        if ( (pal = cthrd.GetPalFromList(ipv4))) {
                /* 既然好友不在线，那么他自然不在列表中 */
                if (!FLAG_ISSET(pal->flags, 1)) {
                        FLAG_SET(pal->flags, 1);
                        gdk_threads_enter();
                        pthread_mutex_lock(cthrd.GetMutex());
                        cthrd.UpdatePalToList(ipv4);
                        pthread_mutex_unlock(cthrd.GetMutex());
                        mwin.AttachItemToPaltree(ipv4);
                        gdk_threads_leave();
                }
        } else {
                SomeoneLost();
                pal = cthrd.GetPalFromList(ipv4);
        }

        return pal;
}

/**
 * 接收好友文件信息.
 */
void UdpData::RecvPalFile()
{
        uint32_t packetno, commandno;
        const char *ptr;
        pthread_t pid;
        GData *para;

        packetno = iptux_get_dec_number(buf, ':', 1);
        commandno = iptux_get_dec_number(buf, ':', 4);
        ptr = iptux_skip_string(buf, size, 1);
        /* 只有当此为共享文件信息或文件信息不为空才需要接收 */
        if ((commandno & IPTUX_SHAREDOPT) || (ptr && *ptr != '\0')) {
                para = NULL;
                g_datalist_init(&para);
                g_datalist_set_data(&para, "palinfo", cthrd.GetPalFromList(ipv4));
                g_datalist_set_data_full(&para, "extra-data", g_strdup(ptr),
                                                 GDestroyNotify(g_free));
                g_datalist_set_data(&para, "packetno", GUINT_TO_POINTER(packetno));
                g_datalist_set_data(&para, "commandno", GUINT_TO_POINTER(commandno));
                pthread_create(&pid, NULL, ThreadFunc(RecvFile::RecvEntry), para);
                pthread_detach(pid);
        }
}

/**
 * 请求获取好友共享文件的密码.
 * @param pal class PalInfo
 */
void UdpData::ThreadAskSharedPasswd(PalInfo *pal)
{
        Command cmd;
        gchar *passwd, *epasswd;

        gdk_threads_enter();
        passwd = pop_obtain_shared_passwd(pal);
        gdk_threads_leave();
        if (passwd && *passwd != '\0') {
                epasswd = g_base64_encode((guchar *)passwd, strlen(passwd));
                cmd.SendAskShared(cthrd.UdpSockQuote(), pal, IPTUX_PASSWDOPT, epasswd);
                g_free(epasswd);
        }
        g_free(passwd);
}

/**
 * 某好友请求本计算机的共享文件.
 * @param pal class PalInfo
 */
void UdpData::ThreadAskSharedFile(PalInfo *pal)
{
        SendFile sfile;
        bool permit;

        if (FLAG_ISSET(progdt.flags, 0)) {
                gdk_threads_enter();
                permit = pop_request_shared_file(pal);
                gdk_threads_leave();
                if (permit)
                        sfile.SendSharedInfoEntry(pal);
        } else
                sfile.SendSharedInfoEntry(pal);
}
