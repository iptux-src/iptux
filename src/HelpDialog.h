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
#ifndef HelpDialog_H
#define HelpDialog_H

#include "deplib.h"

class HelpDialog {
public:
        HelpDialog();
        ~HelpDialog();

        static void AboutEntry();
        static void MoreEntry();
private:
        GtkWidget *CreateAboutDialog();
        GtkWidget *CreateMoreDialog();
        void RunHelpDialog(GtkWidget **dialog);

        static GtkWidget *about;
        static GtkWidget *more;
//回调处理部分
private:
        static void DialogOpenEmail(GtkWidget *dialog, const gchar *link);
        static void DialogOpenUrl(GtkWidget *dialog, const gchar *link);
        static void DialogDestroy(GtkWidget **dialog);
};

#endif
