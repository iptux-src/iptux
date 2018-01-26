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
#ifndef IPTUX_DIALOGPEER_H
#define IPTUX_DIALOGPEER_H

#include "iptux/DialogBase.h"
#include "iptux/IptuxConfig.h"
#include "iptux/mess.h"
#include "iptux/MainWindow.h"

namespace iptux {

class DialogPeer : public DialogBase {
 public:
  DialogPeer(MainWindow* mainWindow, GroupInfo *grp, ProgramData &progdt);
  virtual ~DialogPeer();

  static void PeerDialogEntry(MainWindow* mainWindow, GroupInfo *grpinf, ProgramData &progdt);

  virtual void UpdatePalData(PalInfo *pal);
  virtual void InsertPalData(PalInfo *pal);
  virtual void DelPalData(PalInfo *pal);
  virtual void ClearAllPalData();
  virtual GSList *GetSelPal();
  static void ShowDialogPeer(DialogPeer *dlgpr);
  void insertPicture();

 private:
  MainWindow* mainWindow;

 private:
  void ReadUILayout();
  void WriteUILayout();

  GtkWindow *CreateMainWindow();
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
  void FillPalInfoToBuffer(GtkTextBuffer *buffer, PalInfo *pal);
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
  static void DialogPeerDestroy(DialogPeer *dlgpr);
  static void onAcceptButtonClicked(DialogPeer *dlgpr);
  static void ThreadRecvFile(FileInfo *file);
  static void ShowInfoEnclosure(DialogPeer *dlgpr);
  static bool UpdataEnclosureRcvUI(DialogPeer *dlgpr);
  static void RemoveSelectedRcv(GtkWidget *widget);
  static gint RcvTreePopup(GtkWidget *treeview, GdkEvent *event);
  static void onActive(DialogPeer& self);
  //线程处理
 private:
  static void ThreadSendTextMsg(MsgPara *para);

 protected:
  IptuxConfig &config;
  int64_t torcvsize;  //总计待接收大小(包括已接收)
  int64_t rcvdsize;   //总计已接收大小
  guint timerrcv;     //接收文件界面更新计时器ID
};

}  // namespace iptux

#endif
