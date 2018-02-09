#ifndef IPTUX_UIMODELS_H
#define IPTUX_UIMODELS_H

#include <gtk/gtk.h>

#include "mess.h"

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


enum class PalTreeModelSortKey {
  NICKNAME,
  IP
};
enum class PalTreeModelColumn {
  CLOSED_EXPANDER,
  OPEN_EXPANDER,
  INFO,
  EXTRAS,
  STYLE,
  COLOR,
  DATA,
  N_COLUMNS
};
typedef GtkTreeModel PalTreeModel;
PalTreeModel* palTreeModelNew();
void palTreeModelSetSortKey(PalTreeModel *model, PalTreeModelSortKey key);
/**
 * 更新群组数据(grpinf)到数据集(model)指定位置(iter).
 * @param model model
 * @param iter iter
 * @param grpinf class GroupInfo
 */
G_DEPRECATED_FOR(palTreeModelFillFromGroupInfo)
void groupInfo2PalTreeModel(GroupInfo *grpinf,
                            PalTreeModel *model,
                            GtkTreeIter *iter,
                            const char* font);

/**
 * 填充群组数据(grpinf)到数据集(model)指定位置(iter).
 * @param model model
 * @param iter iter
 * @param grpinf class GroupInfo
 */
void palTreeModelFillFromGroupInfo(PalTreeModel *model,
                                   GtkTreeIter *iter,
                                   const GroupInfo *grpinf,
                                   const char* font);

}

#endif //IPTUX_UIMODELS_H
