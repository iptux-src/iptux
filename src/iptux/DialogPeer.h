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
#include "iptux-core/IptuxConfig.h"
#include "iptux-core/Models.h"
#include "iptux/MainWindow.h"

namespace iptux {

class DialogPeer : public DialogBase {
 public:
  DialogPeer(MainWindow* mainWindow, GroupInfo *grp, std::shared_ptr<UiProgramData> progdt);
  virtual ~DialogPeer();

  static void PeerDialogEntry(MainWindow* mainWindow, GroupInfo *grpinf, std::shared_ptr<UiProgramData> progdt);

  void UpdatePalData(PalInfo *pal) override;
  void InsertPalData(PalInfo *pal) override;
  void DelPalData(PalInfo *pal) override;
  void ClearAllPalData() override;
  GSList *GetSelPal() override;
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
  GtkWidget *CreateFileMenu() override;
  void FillPalInfoToBuffer(GtkTextBuffer *buffer, PalInfo *pal);
  void BroadcastEnclosureMsg(const std::vector<FileInfo*>& files) override;

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
  std::shared_ptr<IptuxConfig> config;
  int64_t torcvsize;  //总计待接收大小(包括已接收)
  int64_t rcvdsize;   //总计已接收大小
  guint timerrcv;     //接收文件界面更新计时器ID
};

}  // namespace iptux

#endif
