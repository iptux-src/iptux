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

#include "iptux-core/IptuxConfig.h"
#include "iptux-core/Models.h"
#include "iptux/DialogBase.h"
#include "iptux/Application.h"

namespace iptux {

class DialogPeer : public DialogBase {
 public:
  DialogPeer(Application* app, GroupInfo *grp);
  virtual ~DialogPeer();

  static void PeerDialogEntry(Application* app, GroupInfo *grpinf);

  void UpdatePalData(PalInfo *pal) override;
  void InsertPalData(PalInfo *pal) override;
  void DelPalData(PalInfo *pal) override;
  void ClearAllPalData() override;
  GSList *GetSelPal() override;
  static void ShowDialogPeer(DialogPeer *dlgpr);
  void insertPicture();
  GtkWindow* getWindow() override { return GTK_WINDOW(window); }

 private:
  Application* app;

 private:
  void ReadUILayout();
  void WriteUILayout();

  GtkWindow *CreateMainWindow();
  GtkWidget *CreateAllArea();

  GtkWidget *CreateInfoArea();
  GtkWidget *CreateFileArea();
  GtkWidget *CreateFileReceiveArea();
  GtkWidget *CreateFileToReceiveArea();
  GtkWidget *CreateFileReceivedArea();
  GtkWidget *CreateFileToReceiveTree(GtkTreeModel *model);
  GtkTreeModel *CreateFileToReceiveModel();
  GtkWidget *CreateFileReceivedTree(GtkTreeModel *model);
  GtkTreeModel *CreateFileReceivedModel();
  void FillPalInfoToBuffer(GtkTextBuffer *buffer, PalInfo *pal);
  void BroadcastEnclosureMsg(const std::vector<FileInfo*>& files) override;

  bool SendTextMsg() override;
  void FeedbackMsg(const std::vector<ChipData>& dtlist);
  MsgPara *PackageMsg(const std::vector<ChipData>& dtlist);
  //回调处理部分
 private:
  static void onAcceptButtonClicked(DialogPeer *self);
  static void onRefuseButtonClicked(DialogPeer *self);
  static void ShowInfoEnclosure(DialogPeer *dlgpr);
  static bool UpdataEnclosureRcvUI(DialogPeer *dlgpr);
  static gint RcvTreePopup(DialogPeer *self, GdkEvent *event);
  static void onClearChatHistory (void *, void *, DialogPeer& self) {
    self.ClearHistoryTextView();
  }
  static void onInsertPicture (void *, void *, DialogPeer& self) {
    self.insertPicture();
  }
  static void onAttachFile(void*, void*, DialogPeer& self) {
    DialogBase::AttachRegular(&self);
  }
  static void onAttachFolder(void*, void*, DialogPeer& self) {
    DialogBase::AttachFolder(&self);
  }
  static void onRequestSharedResources(void*, void*, DialogPeer& self);
  static void onClose(void*, void*, DialogPeer& self) {
    gtk_widget_destroy(GTK_WIDGET(self.window));
  }

 protected:
  GtkApplicationWindow* window;
  std::shared_ptr<IptuxConfig> config;
  int64_t torcvsize;  //总计待接收大小(包括已接收)
  int64_t rcvdsize;   //总计已接收大小
  guint timerrcv;     //接收文件界面更新计时器ID
};

}  // namespace iptux

#endif
