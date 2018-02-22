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
#ifndef IPTUX_PROGRAMDATA_H
#define IPTUX_PROGRAMDATA_H

#include <gtk/gtk.h>

#include "iptux/ProgramDataCore.h"

namespace iptux {

class ProgramData: public ProgramDataCore {
 public:
  explicit ProgramData(IptuxConfig &config);
  ~ProgramData() override;

  GtkTextTagTable *table;        // tag table

 private:
  void InitSublayer();
  void CreateTagTable();
  void CheckIconTheme();
};
}  // namespace iptux

#endif
