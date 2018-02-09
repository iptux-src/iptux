#include "UiUtils.h"

namespace iptux {

void UiUtils::applyTransFileModel2GtkListStore(
    const TransFileModel &para,
    GtkListStore *list_store,
    GtkTreeIter *iter) {
  gtk_list_store_set(
      list_store, iter,
      0, para.getStatus().c_str(),
      1, para.getTask().c_str(),
      2, para.getPeer().c_str(),
      3, para.getIp().c_str(),
      4, para.getFilename().c_str(),
      5, para.getFileLengthText().c_str(),
      6, para.getFinishedLengthText().c_str(),
      7, int(para.getProgress()),
      8, g_strdup(para.getProgressText().c_str()),
      9, para.getCost().c_str(),
      10, para.getRemain().c_str(),
      11, para.getRate().c_str(),
      12, para.getFilePath().c_str(),
      13, para.getData(),
      15, para.isFinished(),
      -1);
}
}
