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

#include "iptux/global.h"
#include "iptux/UiHelper.h"

namespace iptux {

/**
 * 发送探测数据包.
 */
static bool sendDetectPacket(const char* ipv4Text) {
  in_addr ipv4;
  if (inet_pton(AF_INET, ipv4Text, &ipv4) <= 0) {
    return false;
  }
  g_cthrd->SendDetectPacket(ipv4);
  return true;
}

DetectPal::DetectPal(GtkBuilder* builder, GtkWindow* window) {
  this->detectPalDialog = CHECK_NOTNULL(GTK_DIALOG(
      gtk_builder_get_object(builder, "detect_pal_dialog")));
  gtk_window_set_transient_for(GTK_WINDOW(this->detectPalDialog), window);
  this->detectPalIpv4Entry = CHECK_NOTNULL(GTK_ENTRY(
      gtk_builder_get_object(builder, "detect_pal_ipv4_entry")));
}

void DetectPal::run() {
  bool loop = true;
  while(loop) {
    const char* ipv4Text = nullptr;
    switch(gtk_dialog_run(detectPalDialog)) {
      case GTK_RESPONSE_ACCEPT:
        ipv4Text = gtk_entry_get_text(detectPalIpv4Entry);
        if(sendDetectPacket(ipv4Text)) {
          pop_info(GTK_WIDGET(detectPalDialog), _("The notification has been sent to %s."), ipv4Text);
          gtk_entry_set_text(GTK_ENTRY(detectPalIpv4Entry), "");
        } else {
          pop_warning(GTK_WIDGET(detectPalDialog), _("\nIllegal IP(v4) address: %s!"), ipv4Text);
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
