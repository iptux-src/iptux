#include "UiUtils.h"

#include "iptux/UiModels.h"

namespace iptux {

void UiUtils::applyTransFileModel2GtkListStore(
    const TransFileModel &para,
    GtkListStore *list_store,
    GtkTreeIter *iter) {
  gtk_list_store_set(
      list_store, iter,
      TransModelColumn::STATUS, para.getStatus().c_str(),
      TransModelColumn::TASK, para.getTask().c_str(),
      TransModelColumn::PEER, para.getPeer().c_str(),
      TransModelColumn::IP, para.getIp().c_str(),
      TransModelColumn::FILENAME, para.getFilename().c_str(),
      TransModelColumn::FILE_LENGTH_TEXT, para.getFileLengthText().c_str(),
      TransModelColumn::FINISHED_LENGTH_TEXT, para.getFinishedLengthText().c_str(),
      TransModelColumn::PROGRESS, int(para.getProgress()),
      TransModelColumn::PROGRESS_TEXT, g_strdup(para.getProgressText().c_str()),
      TransModelColumn::COST, para.getCost().c_str(),
      TransModelColumn::REMAIN, para.getRemain().c_str(),
      TransModelColumn::RATE, para.getRate().c_str(),
      TransModelColumn::FILE_PATH, para.getFilePath().c_str(),
      TransModelColumn::DATA, para.getData(),
      TransModelColumn::FINISHED, para.isFinished(),
      -1);
}
}
