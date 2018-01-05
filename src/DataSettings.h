//
// C++ Interface: DataSettings
//
// Description:程序功能、数据设置
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DATASETTINGS_H
#define DATASETTINGS_H

#include "mess.h"

class DataSettings {
public:
        DataSettings();
        ~DataSettings();

        static void ResetDataEntry(GtkWidget *parent);
private:
        void InitSublayer();
        void ClearSublayer();

        GtkWidget *CreateMainDialog(GtkWidget *parent);
        GtkWidget *CreatePersonal();
        GtkWidget *CreateSystem();
        GtkWidget *CreateSound();
        GtkWidget *CreateNetwork();

        void SetPersonalValue();
        void SetSystemValue();
        void SetSoundValue();
        void SetNetworkValue();

        GtkTreeModel *CreateIconModel();
        GtkTreeModel *CreateSndModel();
        GtkTreeModel *CreateNetworkModel();
        void FillIconModel(GtkTreeModel *model);
        void FillSndModel(GtkTreeModel *model);
        void FillNetworkModel(GtkTreeModel *model);
        GtkWidget *CreateIconTree(GtkTreeModel *model);
        GtkWidget *CreateSndTree(GtkTreeModel *model);
        GtkWidget *CreateNetworkTree(GtkTreeModel *model);

        GtkWidget *CreateArchiveChooser();
        GtkWidget *CreateFontChooser();
        GtkWidget *CreateSndChooser();

        GData *widset;          //窗体集
        GData *mdlset;          //数据model集
private:
        void ObtainPersonalValue();
        void ObtainSystemValue();
        void ObtainSoundValue();
        void ObtainNetworkValue();

        void WriteNetSegment(const char *filename, GSList *list);
        void ReadNetSegment(const char *filename, GSList **list);

        static GtkWidget *CreatePopupMenu(GtkTreeModel *model);
        static gint IconfileGetItemPos(GtkTreeModel *model, const char *pathname);
//回调处理部分
private:
        static gboolean PopupPickMenu(GtkWidget *treeview, GdkEventButton *event);
        static void AddNewIcon(GtkWidget *button, GData **widset);
        static void ChoosePhoto(GData **widset);

        static void AdjustSensitive(GtkWidget *chkbutton, GtkWidget *widget);
        static void AdjustVolume(GtkWidget *hscale);
        static void SndtreeSelectItemChanged(GtkTreeSelection *selection, GData **widset);
        static void ChooserResetSndtree(GtkWidget *chooser, GData **widset);
        static void PlayTesting(GData **widset);
        static void StopTesting();

        static gint NetworkTreeCompareFunc(GtkTreeModel *model,
                                 GtkTreeIter *a, GtkTreeIter *b);
        static void ClickAddIpseg(GData **widset);
        static void ClickDelIpseg(GData **widset);
        static void CellEditText(GtkCellRendererText *renderer, gchar *path,
                                 gchar *newtext, GtkTreeModel *model);
        static void ImportNetSegment(DataSettings *dset);
        static void ExportNetSegment(DataSettings *dset);
        static void ClearNetSegment(GData **mdlset);
};

#endif
