//
// C++ Implementation: DetectPal
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "config.h"
#include "DetectPal.h"

#include <glib/gi18n.h>
#include <glog/logging.h>

#include "iptux-core/Exception.h"
#include "iptux/UiCoreThread.h"
#include "iptux/UiHelper.h"
#include "iptux/callback.h"

using namespace std;

namespace iptux {

static gboolean iptux_on_detect_pal_ipv4_entry_escape(GtkWidget* widget,
                                                      GdkEventKey* event,
                                                      gpointer) {
  if (event->keyval == GDK_KEY_Escape) {
    gtk_dialog_response(GTK_DIALOG(gtk_widget_get_toplevel(widget)),
                        GTK_RESPONSE_CLOSE);
  }
  return FALSE;
}

DetectPal::DetectPal(Application* app, GtkBuilder* builder, GtkWindow* window)
    : app(app) {
  this->detectPalDialog = CHECK_NOTNULL(
      GTK_DIALOG(gtk_builder_get_object(builder, "detect_pal_dialog")));
  gtk_window_set_transient_for(GTK_WINDOW(this->detectPalDialog), window);
  this->detectPalIpv4Entry = CHECK_NOTNULL(
      GTK_ENTRY(gtk_builder_get_object(builder, "detect_pal_ipv4_entry")));
  g_signal_connect(detectPalIpv4Entry, "insert-text",
                   G_CALLBACK(entry_insert_numeric), nullptr);
  g_signal_connect(detectPalIpv4Entry, "key-release-event",
                   G_CALLBACK(iptux_on_detect_pal_ipv4_entry_escape), nullptr);
}

void DetectPal::run() {
  bool loop = true;
  while (loop) {
    const char* ipv4Text = nullptr;
    switch (gtk_dialog_run(detectPalDialog)) {
      case GTK_RESPONSE_ACCEPT:
        ipv4Text = gtk_entry_get_text(detectPalIpv4Entry);
        try {
          app->getCoreThread()->SendDetectPacket(ipv4Text);
          pop_info(GTK_WIDGET(detectPalDialog),
                   _("The notification has been sent to %s."), ipv4Text);
          gtk_entry_set_text(GTK_ENTRY(detectPalIpv4Entry), "");
        } catch (Exception& e) {
          if (e.getErrorCode() == INVALID_IP_ADDRESS) {
            pop_warning(GTK_WIDGET(detectPalDialog),
                        _("\nIllegal IP(v4) address: %s!"), ipv4Text);
          } else {
            throw e;
          }
        }
        break;
      default:
        loop = false;
        break;
    }
  }
  gtk_widget_hide(GTK_WIDGET(detectPalDialog));
}

}  // namespace iptux
