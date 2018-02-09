#ifndef IPTUX_UIMODELS_H
#define IPTUX_UIMODELS_H

#include <gtk/gtk.h>

namespace iptux {

typedef GtkTreeModel TransModel;
TransModel* trans_model_new();
enum class TransModelColumn {
  STATUS, TASK, PEER, IP, FILENAME,
  FILE_LENGTH_TEXT, FINISHED_LENGTH_TEXT, PROGRESS, PROGRESS_TEXT, COST,
  REMAIN, RATE, FILE_PATH, DATA, PARA,
  FINISHED,
  N_COLUMNS
};




}

#endif //IPTUX_UIMODELS_H
