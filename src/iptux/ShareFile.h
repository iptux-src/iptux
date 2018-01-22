//
// C++ Interface: ShareFile
//
// Description:
// 添加或删除共享文件,即管理共享文件
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_SHAREFILE_H
#define IPTUX_SHAREFILE_H

#include "iptux/mess.h"

namespace iptux {

class ShareFile {
 public:
  ~ShareFile();

  static ShareFile* newShareFile(GtkWidget* parent);

  void run();

 private:
  ShareFile();

  void InitSublayer();
  void ClearSublayer();

  GtkWidget *CreateMainDialog(GtkWidget *parent);
  GtkWidget *CreateAllArea();

  GtkTreeModel *CreateFileModel();
  void FillFileModel(GtkTreeModel *model);
  GtkWidget *CreateFileTree(GtkTreeModel *model);

  void ApplySharedData();
  void AttachSharedFiles(GSList *list);
  GSList *PickSharedFile(uint32_t fileattr);

private:
  GtkWidget* dialog;

  GData *widset;
  GData *mdlset;
  //回调处理部分
 private:
  static void AddRegular(ShareFile *sfile);
  static void AddFolder(ShareFile *sfile);
  static void DeleteFiles(GData **widset);
  static void SetPassword(GData **widset);
  static void ClearPassword(GData **widset);

  static void DragDataReceived(ShareFile *sfile, GdkDragContext *context,
                               gint x, gint y, GtkSelectionData *data,
                               guint info, guint time);
  static gint FileTreeCompareFunc(GtkTreeModel *model, GtkTreeIter *a,
                                  GtkTreeIter *b);
};

}  // namespace iptux

#endif
