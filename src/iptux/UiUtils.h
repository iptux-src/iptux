#ifndef IPTUX_UIUTILS_H
#define IPTUX_UIUTILS_H

#include <gtk/gtk.h>

#include "iptux/TransFileModel.h"

namespace iptux {

class UiUtils {
 public:
  static void applyTransFileModel2GtkListStore(
      const TransFileModel&,
      GtkListStore *list_store,
      GtkTreeIter  *iter
  );
};

}

#endif //IPTUX_UIUTILS_H
