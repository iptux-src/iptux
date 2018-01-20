//
// C++ Interface: DetectPal
//
// Description:探测好友
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_DETECTPAL_H
#define IPTUX_DETECTPAL_H

#include <gtk/gtk.h>

namespace iptux {

class DetectPal {
 public:
  DetectPal();
  ~DetectPal();

  static void DetectEntry(GtkWidget *parent);

 private:
  GtkWidget *CreateMainDialog(GtkWidget *parent);
  GtkWidget *CreateInputArea();
  void SendDetectPacket();

  GData *widset;
};

}  // namespace iptux

#endif
