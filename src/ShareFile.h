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
#ifndef SHAREFILE_H
#define SHAREFILE_H

#include "mess.h"

class ShareFile {
public:
        ShareFile();
        ~ShareFile();

        static void ShareEntry(GtkWidget *parent);
private:
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

#endif
