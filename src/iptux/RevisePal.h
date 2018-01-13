//
// C++ Interface: RevisePal
//
// Description:手动更改好友数据
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef REVISEPAL_H
#define REVISEPAL_H

#include "mess.h"

class RevisePal {
public:
        RevisePal(PalInfo *pl);
        ~RevisePal();

        static void ReviseEntry(PalInfo *pal);
private:
        void InitSublayer();
        void ClearSublayer();

        GtkWidget *CreateMainDialog();
        GtkWidget *CreateAllArea();
        void SetAllValue();
        void ApplyReviseData();

        GtkTreeModel *CreateIconModel();
        void FillIconModel(GtkTreeModel *model);
        GtkWidget *CreateIconTree(GtkTreeModel *model);

        GData *widset;
        GData *mdlset;
        PalInfo *pal;
private:
        static gint IconfileGetItemPos(GtkTreeModel *model, const char *pathname);
//回调处理部分
private:
        static void AddNewIcon(GtkWidget *button, GData **widset);
};

#endif
