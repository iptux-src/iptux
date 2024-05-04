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
#ifndef IPTUX_DATASETTINGS_H
#define IPTUX_DATASETTINGS_H

#include <gtk/gtk.h>

#include "iptux-core/Models.h"
#include "iptux/Application.h"

namespace iptux {

class DataSettings {
 public:
  explicit DataSettings(Application* app);
  ~DataSettings();

  static void ResetDataEntry(Application* app, GtkWidget* parent) {
    ResetDataEntry(app, parent, true);
  }
  static void ResetDataEntry(Application* app, GtkWidget* parent, bool run);

 private:
  void InitSublayer();
  void ClearSublayer();

  GtkWidget* CreateMainDialog(GtkWidget* parent);
  GtkWidget* CreatePersonal();
  GtkWidget* CreateSystem();
  GtkWidget* CreateNetwork();

  void SetPersonalValue();
  void SetSystemValue();

  GtkTreeModel* CreateNetworkModel();
  static void FillIconModel(GtkTreeModel* model);
  void FillNetworkModel(GtkTreeModel* model);
  GtkWidget* CreateIconTree(GtkTreeModel* model);
  GtkWidget* CreateNetworkTree(GtkTreeModel* model);

  GtkWidget* CreateArchiveChooser();
  GtkWidget* CreateFontChooser();

  Application* app;
  GData* widset;  // 窗体集
  GData* mdlset;  // 数据model集
  IconModel* iconModel = 0;
  bool need_restart = false;

 private:
  void ObtainPersonalValue();
  void ObtainSystemValue();
  void ObtainNetworkValue();

  void WriteNetSegment(const char* filename, GSList* list);
  void ReadNetSegment(const char* filename, GSList** list);

  static gint IconfileGetItemPos(GtkTreeModel* model, const char* pathname);

  // 回调处理部分
 private:
  static void AddNewIcon(GtkWidget* button, GData** widset);
  static void ChoosePhoto(GData** widset);

  static void AdjustSensitive(GtkWidget* chkbutton, GtkWidget* widget);

  static gint NetworkTreeCompareFunc(GtkTreeModel* model,
                                     GtkTreeIter* a,
                                     GtkTreeIter* b);
  static void ClickAddIpseg(GData** widset);
  static void ClickDelIpseg(GData** widset);
  static void CellEditText(GtkCellRendererText* renderer,
                           gchar* path,
                           gchar* newtext,
                           GtkTreeModel* model);
  static void ImportNetSegment(DataSettings* dset);
  static void ExportNetSegment(DataSettings* dset);
  static void ClearNetSegment(GData** mdlset);
};

}  // namespace iptux

#endif
