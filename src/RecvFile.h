//
// C++ Interface: RecvFile
//
// Description:
// 接受相关的文件信息,不包含文件数据
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef RECVFILE_H
#define RECVFILE_H

#include "mess.h"

class RecvFile {
public:
        RecvFile();
        ~RecvFile();

        static void RecvEntry(GData *para);
private:
        void InitSublayer();
        void ClearSublayer();
        void ReadUILayout();
        void WriteUILayout();

        void ParseFilePara(GData **para);
        FileInfo *DivideFileinfo(char **extra);
        GtkWidget *CreateWindow(GData **para);
        GtkWidget *CreateAllArea();

        GtkTreeModel *CreateFileModel();
        void FillFileModel(GtkTreeModel *model);
        GtkWidget *CreateFileTree(GtkTreeModel *model);

        GtkWidget *CreateArchiveChooser();

        GData *widset;          //窗体集
        GData *mdlset;          //数据model集
        GData *dtset;           //通用数据集
        GSList *filelist;               //文件链表
private:
        static GtkWidget *CreatePopupMenu(GtkTreeModel *model);
//回调处理部分
private:
        static gboolean PopupPickMenu(GtkWidget *treeview, GdkEventButton *event);
        static void CellEditText(GtkCellRendererText *renderer, gchar *path,
                                         gchar *newtext, GtkTreeModel *model);
        static void FiletreeSelectItemChanged(GtkTreeSelection *selection,
                                                         GData **widset);
        static void ChooserResetStatelabel(GtkWidget *chooser, GData **widset);
        static void ChooserResetFiletree(GtkWidget *chooser, GData **widset);
        static void AttachRecvFile(GData **widset);

        static gboolean WindowConfigureEvent(GtkWidget *window,
                                 GdkEventConfigure *event, GData **dtset);
        static void DialogDestroy(RecvFile *rfile);

        static void ThreadRecvFile(FileInfo *file);
};

#endif
