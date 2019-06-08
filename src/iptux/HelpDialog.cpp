//
// C++ Implementation: HelpDialog
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//          Jally <jallyx@163.com> & ManPT <pentie@gmail.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "config.h"
#include "HelpDialog.h"

#include "iptux-core/deplib.h"
#include "iptux-core/support.h"
#include "iptux/UiHelper.h"

namespace iptux {

static gboolean onActivateLink(GtkAboutDialog *label,
                               gchar          *uri,
                               gpointer        user_data) {
  iptux_open_url(uri);
  return true;
}

HelpDialog::HelpDialog() {}

HelpDialog::~HelpDialog() {}

/**
 * 关于对话框入口.
 */
GtkWidget* HelpDialog::AboutEntry(GtkWindow* parent) {
  HelpDialog hlp;
  return hlp.CreateAboutDialog(parent);
}

/**
 * 创建关于对话框.
 */
GtkWidget *HelpDialog::CreateAboutDialog(GtkWindow* parent) {
  const char *authors[] = {
      "Jally <jallyx@163.com>",
      "ManPT <pentie@gmail.com>",
      "LI Daobing <lidaobing@gmail.com>",
      nullptr
  };
  const char *artists[] = {
      "Jally <jallyx@163.com>",
      "LiWeijian <weijian_li88@qq.com>",
      "ManPT <pentie@gmail.com>",
      nullptr
  };
  const char* credits[] = {
      "ChenGang",
      "<liangsuilong@gmail.com>",
      "<mdjhu@sina.com>",
      "<omegao.hu@gmail.com>",
      "<syranosun@gmail.com>",
      nullptr,
  };

  const char *translator = _("TRANSLATOR NAME");

  const char* origin_translator = "TRANSLATOR NAME";

  GtkWidget *dialog;

  dialog = gtk_about_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
  gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), true);
  gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), _("iptux"));
  gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), VERSION);
  gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog),
                                 "Copyright © 2008-2009 by Jally");
  gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog),
                                _("A GTK+ based LAN Messenger."));
  gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog),
                               "https://github.com/iptux-src/iptux");
  gtk_about_dialog_set_license_type(GTK_ABOUT_DIALOG(dialog), GTK_LICENSE_GPL_2_0);
  gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog), authors);
  gtk_about_dialog_set_artists(GTK_ABOUT_DIALOG(dialog), artists);
  if(strcmp(translator, origin_translator) != 0) {
    gtk_about_dialog_set_translator_credits(GTK_ABOUT_DIALOG(dialog),
                                            translator);
  }
  gtk_about_dialog_add_credit_section(GTK_ABOUT_DIALOG(dialog), _("Thanks to"), credits);
  gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(dialog), "iptux");
  g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), NULL);
  g_signal_connect(dialog, "activate-link", G_CALLBACK(onActivateLink), NULL);
  return dialog;
}

}  // namespace iptux
