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
#include "AboutDialog.h"

#include <glib/gi18n.h>
#include <glog/logging.h>
#include <string>

#include "iptux/UiHelper.h"

using namespace std;

namespace iptux {

static gboolean iptux_on_activate_link(GtkAboutDialog*, gchar* uri, gpointer) {
  iptux_open_url(uri);
  return TRUE;
}

/**
 * 创建关于对话框.
 */
void CreateAboutDialog(AboutDialog* dialog) {
  if (g_object_get_data(G_OBJECT(dialog), "inited")) {
    return;
  }
  g_object_set_data(G_OBJECT(dialog), "inited", GINT_TO_POINTER(TRUE));
  // gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
  gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), VERSION);
  const char* translator = _("TRANSLATOR NAME");
  string originTranslator = "TRANSLATOR NAME";
  if (originTranslator != translator) {
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
  gtk_about_dialog_add_credit_section(GTK_ABOUT_DIALOG(dialog), _("Thanks to"),
                                      credits);
  g_signal_connect(dialog, "activate-link", G_CALLBACK(iptux_on_activate_link),
                   nullptr);
}

/**
 * 关于对话框入口.
 */
AboutDialog* aboutDialogNew() {
  auto builder = gtk_builder_new_from_resource(IPTUX_RESOURCE "gtk/main.ui");
  gtk_builder_connect_signals(builder, nullptr);
  auto aboutDialog = GTK_ABOUT_DIALOG(
      CHECK_NOTNULL(gtk_builder_get_object(builder, "about_dialog")));
  g_object_unref(builder);

  CreateAboutDialog(aboutDialog);
  return aboutDialog;
}

void aboutDialogRun(AboutDialog* aboutDialog, GtkWindow* parent) {
  gtk_window_set_transient_for(GTK_WINDOW(aboutDialog), parent);
  gtk_dialog_run(GTK_DIALOG(aboutDialog));
  gtk_widget_hide(GTK_WIDGET(aboutDialog));
}

void aboutDialogEntry(GtkWindow* parent) {
  AboutDialog* aboutDialog = aboutDialogNew();
  g_object_ref_sink(G_OBJECT(aboutDialog));
  aboutDialogRun(aboutDialog, parent);
  g_object_unref(aboutDialog);
}

}  // namespace iptux
