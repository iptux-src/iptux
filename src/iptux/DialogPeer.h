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
#include "iptux/Models.h"
#include "iptux/MainWindow.h"

namespace iptux {

class DialogPeer : public DialogBase {
 public:
  DialogPeer(MainWindow* mainWindow, GroupInfo *grp, UiProgramData &progdt);
  virtual ~DialogPeer();

  static void PeerDialogEntry(MainWindow* mainWindow, GroupInfo *grpinf, UiProgramData &progdt);

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
  void FeedbackMsg(const std::vector<ChipData>& dtlist);
  MsgPara *PackageMsg(const std::vector<ChipData>& dtlist);
  //回调处理部分
 private:
  static void AskSharedFiles(GroupInfo *grpinf);
  static void onAcceptButtonClicked(DialogPeer *self);
  static void onRefuseButtonClicked(DialogPeer *self);
  static void ThreadRecvFile(FileInfo *file);
  static void ShowInfoEnclosure(DialogPeer *dlgpr);
  static bool UpdataEnclosureRcvUI(DialogPeer *dlgpr);
  static gint RcvTreePopup(DialogPeer *self, GdkEvent *event);
  static void onActive(DialogPeer& self);
 protected:
  IptuxConfig &config;
  int64_t torcvsize;  //总计待接收大小(包括已接收)
  int64_t rcvdsize;   //总计已接收大小
  guint timerrcv;     //接收文件界面更新计时器ID
};

}  // namespace iptux

#endif
