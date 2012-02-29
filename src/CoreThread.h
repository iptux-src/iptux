//
// C++ Interface: CoreThread
//
// Description:
// 程序中的核心线程类，实际上也被设计成了所有底层核心数据的中心点，
// 所有数据的更新、查询、插入、删除都必须通过本类接口才能完成。
// -----------------------------------------------------
//2012.02:把文件传送的核心数据全部放在CoreThread类。
// prlist不变,增加ecsList来存放好友发来文件.
//------------------------------------------------------
// Author: cwll <cwll2009@126.com>, (C) 2012
//         Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CORETHREAD_H
#define CORETHREAD_H

#include "mess.h"

/**
 * @note 请保证插入或更新某成员时，底层优先于UI；删除某成员时，UI优先于底层，
 * 否则你会把所有事情都搞砸. \n
 * @note 鉴于(GroupInfo::member)成员发生变动时必须保证函数处于UI线程安全的环境，
 * 所以UI线程安全的函数对(GroupInfo::member)的访问无须加锁.\n
 * 若此特性不可被如此利用，请报告bug. \n
 * @note 如果本程序编码中的某处没有遵循以上规则，请报告bug.
 */
class CoreThread {
public:
        CoreThread();
        ~CoreThread();

        void CoreThreadEntry();
        void WriteSharedData();
        GSList *GetPalList();
        pthread_mutex_t *GetMutex();

        void InsertMessage(MsgPara *para);
        void InsertMsgToGroupInfoItem(GroupInfo *grpinf, MsgPara *para);
        static void SendNotifyToAll(CoreThread *pcthrd);
        static void SendFeatureData(PalInfo *pal);
        static void SendBroadcastExit(PalInfo *pal);
        static void UpdateMyInfo();

        void ClearAllPalFromList();
        PalInfo *GetPalFromList(in_addr_t ipv4);
        bool ListContainPal(in_addr_t ipv4);
        void DelPalFromList(in_addr_t ipv4);
        void UpdatePalToList(in_addr_t ipv4);
        void AttachPalToList(PalInfo *pal);
        GroupInfo *GetPalRegularItem(PalInfo *pal);
        GroupInfo *GetPalSegmentItem(PalInfo *pal);
        GroupInfo *GetPalGroupItem(PalInfo *pal);
        GroupInfo *GetPalBroadcastItem(PalInfo *pal);

        bool BlacklistContainItem(in_addr_t ipv4);
        void AttachItemToBlacklist(in_addr_t ipv4);
        void ClearBlacklist();

        guint GetMsglineItems();
        GroupInfo *GetMsglineHeadItem();
        bool MsglineContainItem(GroupInfo *grpinf);
        void PushItemToMsgline(GroupInfo *grpinf);
        void PopItemFromMsgline(GroupInfo *grpinf);

        GSList *GetPalEnclosure(PalInfo *pal);
        void PushItemToEnclosureList(FileInfo *file);
        void PopItemFromEnclosureList(FileInfo *file);
        GSList *GetPalRcvdEnclosure(PalInfo *pal);

        void AttachFileToPublic(FileInfo *file);
        void DelFileFromPublic(uint32_t fileid);
        void ClearFileFromPublic();
        GSList *GetPublicFileList();
        void AttachFileToPrivate(FileInfo *file);
        void DelFileFromPrivate(uint32_t fileid);
        void ClearFileFromPrivate();
        FileInfo *GetFileFromAll(uint32_t fileid);
        FileInfo *GetFileFromAllWithPacketN(uint32_t packageNum,uint32_t filectime);
        const char *GetAccessPublicLimit();
        void SetAccessPublicLimit(const char *limit);
private:
        void InitSublayer();
        void ClearSublayer();
        void InitThemeSublayerData();
        void ReadSharedData();

        void InsertHeaderToBuffer(GtkTextBuffer *buffer, MsgPara *para);
        void InsertStringToBuffer(GtkTextBuffer *buffer, gchar *string);
        void InsertPixbufToBuffer(GtkTextBuffer *buffer, gchar *path);

        GroupInfo *GetPalPrevGroupItem(PalInfo *pal);
        GroupInfo *AttachPalRegularItem(PalInfo *pal);
        GroupInfo *AttachPalSegmentItem(PalInfo *pal);
        GroupInfo *AttachPalGroupItem(PalInfo *pal);
        GroupInfo *AttachPalBroadcastItem(PalInfo *pal);
        void DelPalFromGroupInfoItem(GroupInfo *grpinf, PalInfo *pal);
        void AttachPalToGroupInfoItem(GroupInfo *grpinf, PalInfo *pal);

        int tcpsock, udpsock;   //程序的服务监听套接口
        bool server;            //程序是否正在服务

        GSList *pallist;                //好友链表(成员不能被删除)
        GSList *rgllist, *sgmlist, *grplist, *brdlist;  //群组链表(成员不能被删除)
        GSList *blacklist;      //黑名单链表
        GQueue msgline; //消息队列

        uint32_t pbn, prn;      //当前已使用的文件编号(共享/私有)
        GSList *pblist, *prlist;        //文件链表(共享/私有)
        GSList *ecsList;                //文件链表(好友发过来)
//        GSList *rcvdList;               //文件链表(好友发过来已接收)
        char *passwd;           //共享文件密码

        guint timerid;  //定时器ID
        pthread_mutex_t mutex;  //锁
//回调处理部分函数
private:
        static void RecvUdpData(CoreThread *pcthrd);
        static void RecvTcpData(CoreThread *pcthrd);
        static gboolean WatchCoreStatus(CoreThread *pcthrd);
//内联成员函数
public:
        inline int &TcpSockQuote() {
                return tcpsock;
        } inline int &UdpSockQuote() {
                return udpsock;
        } inline uint32_t &PbnQuote() {
                return pbn;
        } inline uint32_t &PrnQuote() {
                return prn;
        }
};

#endif
