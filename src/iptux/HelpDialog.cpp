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

#include <glib/gi18n.h>


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
GtkWidget* HelpDialog::AboutEntry(GtkAboutDialog* dialog, GtkWindow* parent) {
  HelpDialog hlp;
  return hlp.CreateAboutDialog(dialog, parent);
}

/**
 * 创建关于对话框.
 */
GtkWidget *HelpDialog::CreateAboutDialog(GtkAboutDialog* dialog, GtkWindow* parent) {
  if(g_object_get_data(G_OBJECT(dialog), "inited")) {
    return GTK_WIDGET(dialog);
  }
  g_object_set_data(G_OBJECT(dialog), "inited", GINT_TO_POINTER(TRUE));
  gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
  gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), VERSION);
  const char *translator = _("TRANSLATOR NAME");
  const char* origin_translator = "TRANSLATOR NAME";
  if(strcmp(translator, origin_translator) != 0) {
    gtk_about_dialog_set_translator_credits(GTK_ABOUT_DIALOG(dialog),
                                            translator);
  }
  const char* credits[] = {
      "ChenGang",
      "<liangsuilong@gmail.com>",
      "<mdjhu@sina.com>",
      "<omegao.hu@gmail.com>",
      "<syranosun@gmail.com>",
      nullptr,
  };
  gtk_about_dialog_add_credit_section(GTK_ABOUT_DIALOG(dialog), _("Thanks to"), credits);
  // g_signal_connect(dialog, "activate-link", G_CALLBACK(onActivateLink), NULL);

  return GTK_WIDGET(dialog);
}

}  // namespace iptux
