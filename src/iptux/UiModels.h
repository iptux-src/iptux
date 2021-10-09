#ifndef IPTUX_UIMODELS_H
#define IPTUX_UIMODELS_H

#include <gtk/gtk.h>

#include "iptux-core/Models.h"
#include "iptux-core/TransFileModel.h"
#include "iptux/GroupInfo.h"
#include "iptux/LogSystem.h"

namespace iptux {

typedef void (*GActionCallback)(GSimpleAction* action,
                                GVariant* parameter,
                                gpointer user_data);
#define G_ACTION_CALLBACK(f) ((GActionCallback)(f))

/**
 * 会话抽象类.
 * 提供好友会话类必需的公共接口.
 */
class SessionAbstract {
 public:
  SessionAbstract(){};
  virtual ~SessionAbstract(){};

  virtual void UpdatePalData(PalInfo* pal) = 0;  ///< 更新好友数据
  virtual void InsertPalData(PalInfo* pal) = 0;  ///< 插入好友数据
  virtual void DelPalData(PalInfo* pal) = 0;     ///< 删除好友数据
  virtual void ClearAllPalData() = 0;            ///< 清除所有好友数据
  //        virtual void ShowEnclosure() = 0;               ///< 显示附件
  virtual void AttachEnclosure(const GSList* list) = 0;  ///< 添加附件
  virtual void OnNewMessageComing() = 0;  ///< 窗口打开情况下有新消息
};

enum class TransModelColumn {
  STATUS,
  TASK,
  PEER,
  IP,
  FILENAME,
  FILE_LENGTH_TEXT,
  FINISHED_LENGTH_TEXT,
  PROGRESS,
  PROGRESS_TEXT,
  COST,
  REMAIN,
  RATE,
  FILE_PATH,
  FINISHED,
  TASK_ID,
  N_COLUMNS
};
typedef GtkTreeModel TransModel;
TransModel* transModelNew();
void transModelDelete(TransModel*);
void transModelUpdateFromTransFileModel(TransModel* model,
                                        const TransFileModel&);
void transModelLoadFromTransFileModels(
    TransModel* model,
    const std::vector<std::unique_ptr<TransFileModel>>& fileModels);
bool transModelIsFinished(TransModel*);

enum class PalTreeModelSortKey { NICKNAME, IP };
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
void palTreeModelSetSortKey(PalTreeModel* model, PalTreeModelSortKey key);
/**
 * 填充群组数据(grpinf)到数据集(model)指定位置(iter).
 * @param model model
 * @param iter iter
 * @param grpinf class GroupInfo
 */
void palTreeModelFillFromGroupInfo(PalTreeModel* model,
                                   GtkTreeIter* iter,
                                   const GroupInfo* grpinf,
                                   const char* font);

/**
 * 更新群组数据(grpinf)到数据集(model)指定位置(iter).
 * @param model model
 * @param iter iter
 * @param grpinf class GroupInfo
 */
G_DEPRECATED_FOR(palTreeModelFillFromGroupInfo)
void groupInfo2PalTreeModel(GroupInfo* grpinf,
                            PalTreeModel* model,
                            GtkTreeIter* iter,
                            const char* font);

enum class IconModelColumn { ICON, ICON_NAME, N_COLUMNS };
typedef GtkListStore IconModel;
IconModel* iconModelNew();

}  // namespace iptux

#endif  // IPTUX_UIMODELS_H
