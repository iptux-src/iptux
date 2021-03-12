//
// C++ Interface: DialogBase
//
// Description:
// 这个类是DialogPeer和DialogGroup的相同部分。尽量把相同的部分放在一起。
//
// Author:
// Author: cwll <cwll2009@126.com> ,(C) 2012.02
//         Jiejing.Zhang <kzjeef@gmail.com>, (C) 2010
//         Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_DIALOGBASE_H
#define IPTUX_DIALOGBASE_H

#include <memory>

#include "iptux/UiProgramData.h"
#include "iptux-core/Models.h"
#include "iptux/UiModels.h"

namespace iptux {

class DialogBase : public SessionAbstract {
 public:
  DialogBase(GroupInfo *grp, std::shared_ptr<UiProgramData> progdt);
  virtual ~DialogBase();

  void ClearHistoryTextView();

 protected:
  void InitSublayerGeneral();
  void ClearSublayerGeneral();

  void ScrollHistoryTextview();
  virtual void OnNewMessageComing();
  void NotifyUser();

  void AttachEnclosure(const GSList *list);
  /* UI general */
  GtkWidget *CreateInputArea();
  virtual GtkWidget *CreateHistoryArea();
  virtual GtkWidget *CreateFileSendArea();
  virtual GtkWidget *CreateFileSendTree(GtkTreeModel *model);
  virtual GtkWindow* getWindow() = 0;
  virtual GSList *GetSelPal() { return NULL; };

  void MainWindowSignalSetup(GtkWindow *window);
  GtkTreeModel *CreateFileSendModel();
  GSList *PickEnclosure(FileAttr fileattr);

  bool SendEnclosureMsg();
  virtual bool SendTextMsg() = 0;
  /* TODO: Group SendTextMsg need add Picture */
  void FeedbackMsg(const gchar *msg);
  virtual void BroadcastEnclosureMsg(const std::vector<FileInfo*>& files) = 0;

  // 回调部分
  static void DialogDestory(DialogBase *);
  static gboolean ClearNotify(GtkWidget *window, GdkEventConfigure *event);
  static void DragDataReceived(DialogBase *dlgpr, GdkDragContext *context,
                               gint x, gint y, GtkSelectionData *data,
                               guint info, guint time);
  static void AttachRegular(DialogBase *dlgpr);
  static void AttachFolder(DialogBase *dlgpr);
  static void RemoveSelectedFromTree(GtkWidget *widget);
  static void SendMessage(DialogBase *dlggrp);
  static gboolean WindowConfigureEvent(GtkWidget *window,
                                       GdkEventConfigure *event, GData **dtset);
  static void PanedDivideChanged(GtkWidget *paned, GParamSpec *pspec,
                                 GData **dtset);
  static gint EnclosureTreePopup(GtkWidget *widget, GdkEvent *event);
  static gboolean UpdateFileSendUI(DialogBase *dlggrp);
  static void RemoveSelectedEnclosure(GtkWidget *widget);
  static void OpenTransDlg(DialogBase *dlgpr);

 protected:
  std::shared_ptr<UiProgramData> progdt;
  GData *widset;            //窗体集
  GData *mdlset;            //数据model集
  GData *dtset;             //通用数据集
  GtkAccelGroup *accel;     //快捷键组
  GroupInfo *grpinf;        //群组信息
  int64_t totalsendsize;    //总计待发送大小(包括已发送)
  struct timeval lasktime;  //上一次更新UI的时间
  guint timersend;          // 发送文件界面更新计时器ID
};

}  // namespace iptux

#endif
