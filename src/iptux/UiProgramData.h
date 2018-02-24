//
// C++ Interface: ProgramData
//
// Description: 与iptux相关的程序数据
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_UIPROGRAMDATA_H
#define IPTUX_UIPROGRAMDATA_H

#include <gtk/gtk.h>

#include "iptux/ProgramDataCore.h"

namespace iptux {

class UiProgramData: public ProgramDataCore {
 public:
  explicit UiProgramData(IptuxConfig &config);
  ~UiProgramData() override;

  GtkTextTagTable *table;        // tag table

 private:
  void InitSublayer();
  void CreateTagTable();
  void CheckIconTheme();
};
}  // namespace iptux

#endif
