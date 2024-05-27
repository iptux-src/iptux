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
#include "iptux/Application.h"
#include "iptux/DialogBase.h"

namespace iptux {

class DialogPeer : public DialogBase {
 public:
  DialogPeer(Application* app, GroupInfo* grp);
  virtual ~DialogPeer();

  static void PeerDialogEntry(Application* app, GroupInfo* grpinf);

  void UpdatePalData(PalInfo* pal) override;
  void InsertPalData(PalInfo* pal) override;
  void DelPalData(PalInfo* pal) override;
  void ClearAllPalData() override;
  GSList* GetSelPal() override;
  static void ShowDialogPeer(DialogPeer* dlgpr);
  void insertPicture();
  GtkWindow* getWindow() override { return GTK_WINDOW(window); }

 private:
  GtkTreeView* fileToReceiveTree = 0;

 private:
  void ReadUILayout();
  void WriteUILayout();

  void init();

  GtkWindow* CreateMainWindow();
  void CreateTitle();
  GtkWidget* CreateAllArea();

  GtkWidget* CreateInfoArea();
  GtkWidget* CreateFileArea();
  GtkWidget* CreateFileReceiveArea();
  GtkWidget* CreateFileToReceiveArea();
  GtkWidget* CreateFileReceivedArea();
  GtkWidget* CreateFileToReceiveTree(GtkTreeModel* model);
  GtkTreeModel* CreateFileToReceiveModel();
  GtkWidget* CreateFileReceivedTree(GtkTreeModel* model);
  GtkTreeModel* CreateFileReceivedModel();
  void refreshTitle();
  void FillPalInfoToBuffer(GtkTextBuffer* buffer, PalInfo* pal);
  void BroadcastEnclosureMsg(const std::vector<FileInfo*>& files) override;

  bool SendTextMsg() override;
  void FeedbackMsg(const std::vector<ChipData>& dtlist);
  MsgPara* PackageMsg(const std::vector<ChipData>& dtlist);
  void refreshSendAction();
  std::string GetTitle();

  // 回调处理部分
 private:
  static void onRecvTreeSelectionChanged(DialogPeer& self, GtkTreeSelection*);
  static void onAcceptButtonClicked(DialogPeer* self);
  static void ShowInfoEnclosure(DialogPeer* dlgpr);
  static bool UpdataEnclosureRcvUI(DialogPeer* dlgpr);
  static gint RcvTreePopup(GtkWidget*, GdkEvent* event, DialogPeer* self);
  static void onRefuse(void*, void*, DialogPeer& self);
  static void onRefuseAll(void*, void*, DialogPeer& self);
  void onNewFileReceived(GroupInfo*);
  static void onClearChatHistory(void*, void*, DialogPeer& self) {
    self.ClearHistoryTextView();
  }
  static void onInsertPicture(void*, void*, DialogPeer& self) {
    self.insertPicture();
  }
  static void onAttachFile(void*, void*, DialogPeer& self) {
    DialogBase::AttachRegular(&self);
  }
  static void onAttachFolder(void*, void*, DialogPeer& self) {
    DialogBase::AttachFolder(&self);
  }
  static void onRequestSharedResources(void*, void*, DialogPeer& self);
  static void onPaste(void*, void*, DialogPeer* self);
  static void onSendMessage(void*, void*, DialogPeer& self) {
    DialogBase::SendMessage(&self);
  }
  void onGroupInfoUpdated(GroupInfo* groupInfo);
  static void onInputBufferChanged(GtkTextBuffer*, DialogPeer& self) {
    self.refreshSendAction();
  }
  static void onSendFileModelChanged(DialogPeer& self) {
    self.refreshSendAction();
  }

 protected:
  GtkApplicationWindow* window;
  std::shared_ptr<IptuxConfig> config;
  int64_t torcvsize;  // 总计待接收大小(包括已接收)
  int64_t rcvdsize;   // 总计已接收大小
  guint timerrcv;     // 接收文件界面更新计时器ID
  GtkWidget* fileToReceiveTreeviewWidget = nullptr;
  gulong sigId = 0;
};

}  // namespace iptux

#endif
