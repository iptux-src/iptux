//
// C++ Interface: HelpDialog
//
// Description:
// 创建帮助相关对话框
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_HELP_DIALOG_H
#define IPTUX_HELP_DIALOG_H

#include <gtk/gtk.h>

namespace iptux {

class HelpDialog {
 public:
  HelpDialog();
  ~HelpDialog();

  static void AboutEntry(GtkWindow* parent, bool run);

  static void AboutEntry(GtkWindow* parent) { return AboutEntry(parent, true); }

 private:
  GtkWidget* CreateAboutDialog(GtkAboutDialog* dlg, GtkWindow* parent);
  void RunHelpDialog(GtkWidget** dialog);

  static GtkWidget* about;
  static GtkWidget* more;
  //回调处理部分
 private:
  static void DialogDestroy(GtkWidget** dialog);
};

}  // namespace iptux

#endif
