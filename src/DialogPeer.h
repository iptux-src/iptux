//
// C++ Interface: DialogPeer
//
// Description:
// 与单个好友对话
//
// Author: cwll <cwll2009@126.com> ,(C) 2012.02
//        Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DIALOGPEER_H
#define DIALOGPEER_H

#include "mess.h"
#include "DialogBase.h"

class DialogPeer: public DialogBase {
public:
        DialogPeer(GroupInfo *grp);
        virtual ~DialogPeer();

        static void PeerDialogEntry(GroupInfo *grpinf);

        virtual void UpdatePalData(PalInfo *pal);
        virtual void InsertPalData(PalInfo *pal);
        virtual void DelPalData(PalInfo *pal);
        virtual void ClearAllPalData();
        virtual GSList *GetSelPal();
        static void ShowDialogPeer(DialogPeer *dlgpr);
private:
        void ReadUILayout();
        void WriteUILayout();

        GtkWidget *CreateMainWindow();
        GtkWidget *CreateAllArea();

        GtkWidget *CreateMenuBar();
        GtkWidget *CreateInfoArea();
        GtkWidget *CreateFileArea();
        GtkWidget *CreateFileReceiveArea();
        GtkWidget *CreateFileToReceiveArea();
        GtkWidget *CreateFileReceivedArea();
        GtkWidget *CreateFileToReceiveTree(GtkTreeModel *model);
        GtkTreeModel *CreateFileToReceiveModel();
        GtkWidget *CreateFileReceivedTree(GtkTreeModel *model);
        GtkTreeModel *CreateFileReceivedModel();
        GtkWidget *CreateFileMenu();
        GtkWidget *CreateToolMenu();
        void FillPalInfoToBuffer(GtkTextBuffer *buffer, PalInfo *pal);

private:
        void BroadcastEnclosureMsg(GSList *list);
        bool SendTextMsg();
        void FeedbackMsg(const GSList *dtlist);
        MsgPara *PackageMsg(GSList *dtlist);
//回调处理部分
private:
        static void DragPicReceived(DialogPeer *dlgpr, GdkDragContext *context,
                                         gint x, gint y, GtkSelectionData *data,
                                         guint info, guint time);
        static void AskSharedFiles(GroupInfo *grpinf);
        static void InsertPicture(DialogPeer *dlgpr);
        static void DialogPeerDestroy(DialogPeer *dlgpr);
        static void ReceiveFile(DialogPeer *dlgpr);
        static void ThreadRecvFile(FileInfo *file);
        static void ShowInfoEnclosure(DialogPeer *dlgpr);
        static bool UpdataEnclosureRcvUI(DialogPeer *dlgpr);
        static void RemoveSelectedRcv(GtkWidget *widget);
        static gint RcvTreePopup(GtkWidget *treeview,GdkEvent *event);
//线程处理
private:
        static void ThreadSendTextMsg(MsgPara *para);
protected:
        int64_t torcvsize;  //总计待接收大小(包括已接收)
        int64_t rcvdsize;   //总计已接收大小
        guint timerrcv;     //接收文件界面更新计时器ID
};

#endif
