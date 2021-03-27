#include "config.h"
#include "UiModels.h"

#include <cstring>
#include <glog/logging.h>

#include "iptux-core/Const.h"
#include "iptux-core/Models.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"
#include "iptux/UiHelper.h"

using namespace std;



namespace iptux {

/**
 * 文件传输树(trans-tree)底层数据结构.
 * 14,0 status,1 task,2 peer,3 ip,4 filename,5 filelength,6 finishlength,7
 * progress, 8 pro-text,9 cost,10 remain,11 rate,12,pathname,13 data,14 para, 15 finished
 *
 * 任务状态;任务类型;任务对端;文件名(如果当前是文件夹，该项指正在传输的文件夹内单个文件,
 * 整个文件夹传输完成后,该项指向当前是文件夹);文件长度;完成长度;完成进度;
 * 进度串;已花费时间;任务剩余时间;传输速度;带路径文件名(不显示);文件传输类;参数指针值
 *
 * @return trans-model
 */
TransModel* transModelNew() {
  GtkListStore *model;

  model = gtk_list_store_new(int(TransModelColumn::N_COLUMNS),
                             G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                             G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                             G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING,
                             G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                             G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_POINTER,
                             G_TYPE_BOOLEAN);
  return GTK_TREE_MODEL(model);
}

void transModelDelete(TransModel* model) {
  g_object_unref(model);
}

static void transModelFillFromTransFileModel(TransModel* model, GtkTreeIter* iter, const TransFileModel& para) {
  gtk_list_store_set(
      GTK_LIST_STORE(model), iter,
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
      TransModelColumn::TASK_ID, para.getTaskId(),
      TransModelColumn::FINISHED, para.isFinished(),
      -1);
}

void transModelUpdateFromTransFileModel(TransModel* model, const TransFileModel& transFileModel) {
  GtkTreeIter iter;
  bool found = false;
  if (gtk_tree_model_get_iter_first(model, &iter)) {
    do {
      int taskId = 0;
      gtk_tree_model_get(model, &iter, TransModelColumn::TASK_ID, &taskId, -1);
      if(taskId == transFileModel.getTaskId()) {
        found = true;
        break;
      }
    } while (gtk_tree_model_iter_next(model, &iter));
  }
  if (!found) {
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
  }

  /* 重设数据 */
  transModelFillFromTransFileModel(model, &iter, transFileModel);
}


void transModelLoadFromTransFileModels(TransModel* model, const vector<unique_ptr<TransFileModel>>& fileModels) {
  gtk_list_store_clear(GTK_LIST_STORE(model));
  for(auto& it: fileModels) {
    GtkTreeIter iter;
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    transModelFillFromTransFileModel(model, &iter, *(it.get()));
  }
}




/**
 * 好友树(paltree)按昵称排序的比较函数.
 * @param model paltree-model
 * @param a A GtkTreeIter in model
 * @param b Another GtkTreeIter in model
 * @return 比较值
 */
gint paltreeCompareByNameFunc(GtkTreeModel *model, GtkTreeIter *a,
                              GtkTreeIter *b) {
  GroupInfo *agrpinf, *bgrpinf;
  gint result;

  gtk_tree_model_get(model, a, PalTreeModelColumn::DATA, &agrpinf, -1);
  gtk_tree_model_get(model, b, PalTreeModelColumn::DATA, &bgrpinf, -1);
  result = strcmp(agrpinf->name.c_str(), bgrpinf->name.c_str());

  return result;
}

/**
 * 好友树(paltree)按IP排序的比较函数.
 * @param model paltree-model
 * @param a A GtkTreeIter in model
 * @param b Another GtkTreeIter in model
 * @return 比较值
 */
gint paltreeCompareByIPFunc(GtkTreeModel *model, GtkTreeIter *a,
                                        GtkTreeIter *b) {
  GroupInfo *agrpinf, *bgrpinf;
  gtk_tree_model_get(model, a, PalTreeModelColumn::DATA, &agrpinf, -1);
  gtk_tree_model_get(model, b, PalTreeModelColumn::DATA, &bgrpinf, -1);
  if (agrpinf->getType() == GROUP_BELONG_TYPE_REGULAR &&
      bgrpinf->getType() == GROUP_BELONG_TYPE_REGULAR)
  {
    if(agrpinf->grpid < bgrpinf->grpid) {
      return -1;
    }
    if(agrpinf->grpid == bgrpinf->grpid) {
      return 0;
    }
    return 1;
  }

  return 0;
}



/**
 * 好友树(paltree)底层数据结构.
 * 7,0 closed-expander,1 open-expander,2 info.,3 extras,4 style,5 color,6 data
 * \n 关闭的展开器;打开的展开器;群组信息;扩展信息;字体风格;字体颜色;群组数据 \n
 * @return paltree-model
 */
PalTreeModel * palTreeModelNew() {
  GtkTreeStore *model;

  model = gtk_tree_store_new(int(PalTreeModelColumn::N_COLUMNS),
                             GDK_TYPE_PIXBUF, GDK_TYPE_PIXBUF, G_TYPE_STRING,
                             G_TYPE_STRING, PANGO_TYPE_ATTR_LIST,
                             GDK_TYPE_RGBA, G_TYPE_POINTER);
  palTreeModelSetSortKey(GTK_TREE_MODEL(model), PalTreeModelSortKey::NICKNAME);
  gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model),
                                       GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
                                       GTK_SORT_ASCENDING);

  return GTK_TREE_MODEL(model);
}

void palTreeModelSetSortKey(PalTreeModel *model, PalTreeModelSortKey key) {
  switch(key) {
    case PalTreeModelSortKey::NICKNAME:
      gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(model), GtkTreeIterCompareFunc(paltreeCompareByNameFunc), NULL,
                                              NULL);
      break;
    case PalTreeModelSortKey::IP:
      gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(model), GtkTreeIterCompareFunc(paltreeCompareByIPFunc), NULL,
                                              NULL);
      break;
    default:
      LOG_WARN("unknown PalTreeModelSortKey: %d", key);
  }
}

/**
 * 更新群组数据(grpinf)到数据集(model)指定位置(iter).
 * @param model model
 * @param iter iter
 * @param grpinf class GroupInfo
 */
void groupInfo2PalTreeModel(GroupInfo *grpinf,
                            PalTreeModel *model,
                            GtkTreeIter *iter,
                            const char* font) {
  palTreeModelFillFromGroupInfo(model, iter, grpinf, font);
}

static const GdkRGBA color = {0.3216, 0.7216, 0.2196, 0.0};


/**
 * 填充群组数据(grpinf)到数据集(model)指定位置(iter).
 * @param model model
 * @param iter iter
 * @param grpinf class GroupInfo
 */
void palTreeModelFillFromGroupInfo(GtkTreeModel *model,
                                   GtkTreeIter *iter,
                                   const GroupInfo *grpinf,
                                   const char* font) {
  GtkIconTheme *theme;
  GdkPixbuf *cpixbuf, *opixbuf= nullptr;
  PangoAttrList *attrs;
  PangoAttribute *attr;
  gchar *extra;
  PalInfo *pal;
  GError* error = nullptr;

  /* 创建图标 */
  theme = gtk_icon_theme_get_default();
  if (grpinf->getType() == GROUP_BELONG_TYPE_REGULAR) {
    pal = grpinf->getMembers()[0].get();
    auto file = iptux_erase_filename_suffix(pal->iconfile);
    cpixbuf = gtk_icon_theme_load_icon(theme, file, MAX_ICONSIZE,
                                       GtkIconLookupFlags(0), &error);
    if(cpixbuf == nullptr) {
      LOG_WARN("gtk_icon_theme_load_icon failed: [%d] %s", error->code, error->message);
      g_error_free(error);
      error = nullptr;
    } else {
      opixbuf = GDK_PIXBUF(g_object_ref(cpixbuf));
    }
    g_free(file);
  } else {
    cpixbuf = gtk_icon_theme_load_icon(theme, "tip-hide", MAX_ICONSIZE,
                                       GtkIconLookupFlags(0), NULL);
    opixbuf = gtk_icon_theme_load_icon(theme, "tip-show", MAX_ICONSIZE,
                                       GtkIconLookupFlags(0), NULL);
  }

  string info;
  /* 创建主信息 */
  if (grpinf->getType() == GROUP_BELONG_TYPE_REGULAR) {
    char ipstr[INET_ADDRSTRLEN];
    pal = grpinf->getMembers()[0].get();
    inet_ntop(AF_INET, &pal->ipv4, ipstr, INET_ADDRSTRLEN);
    int unreadMsgCount = grpinf->getUnreadMsgCount();
    if(unreadMsgCount > 0) {
      info = stringFormat("%s <span foreground=\"red\">(%d)</span>\n%s",
        markupEscapeText(pal->name).c_str(),
        unreadMsgCount,
        markupEscapeText(ipstr).c_str());
    } else {
      info = stringFormat("%s\n%s",
        markupEscapeText(pal->name).c_str(),
        markupEscapeText(ipstr).c_str());
    }
  } else
    info = markupEscapeText(grpinf->name);

  /* 创建扩展信息 */
  if (grpinf->getType() == GROUP_BELONG_TYPE_REGULAR)
    extra = NULL;
  else
    extra = g_strdup_printf("(%d)", (int)(grpinf->getMembers().size()));

  /* 创建字体风格 */
  attrs = pango_attr_list_new();
  if (grpinf->getType() == GROUP_BELONG_TYPE_REGULAR) {
    auto dspt = pango_font_description_from_string(font);
    attr = pango_attr_font_desc_new(dspt);
    pango_attr_list_insert(attrs, attr);
    pango_font_description_free(dspt);
  } else {
    attr = pango_attr_size_new(8192);
    pango_attr_list_insert(attrs, attr);
    attr = pango_attr_style_new(PANGO_STYLE_ITALIC);
    pango_attr_list_insert(attrs, attr);
    attr = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
    pango_attr_list_insert(attrs, attr);
  }

  /* 设置相应的数据 */
  gtk_tree_store_set(GTK_TREE_STORE(model), iter,
                     PalTreeModelColumn ::CLOSED_EXPANDER, cpixbuf,
                     PalTreeModelColumn ::OPEN_EXPANDER, opixbuf,
                     PalTreeModelColumn ::INFO, info.c_str(),
                     PalTreeModelColumn ::EXTRAS, extra,
                     PalTreeModelColumn ::STYLE, attrs,
                     PalTreeModelColumn ::COLOR, &color,
                     PalTreeModelColumn ::DATA, grpinf,
                     -1);

  /* 释放资源 */
  if (cpixbuf) g_object_unref(cpixbuf);
  if (opixbuf) g_object_unref(opixbuf);
  g_free(extra);
  pango_attr_list_unref(attrs);
}

// GroupInfo::GroupInfo()
//     : grpid(0),
//       type(GROUP_BELONG_TYPE_REGULAR),
//       member(NULL),
//       buffer(NULL),
//       dialog(NULL) {}
GroupInfo::~GroupInfo() {
  g_object_unref(action);
  g_object_unref(buffer);
}

bool GroupInfo::hasPal(PalInfo* pal) const {
  for(auto i: members) {
    if(i.get() == pal) {
      return true;
    }
  }
  return false;
}

bool GroupInfo::hasPal(PPalInfo pal) const {
  return hasPal(pal.get());
}

GroupInfo::GroupInfo(PPalInfo pal)
    : grpid(0),
      buffer(NULL),
      dialog(NULL),
      type(GROUP_BELONG_TYPE_REGULAR)
{
  action = g_simple_action_new("default", G_VARIANT_TYPE_STRING);
  members.push_back(pal);
}

GroupInfo::GroupInfo(iptux::GroupBelongType t, const vector<PPalInfo>& pals)
    : grpid(0),
      buffer(NULL),
      dialog(NULL),
      members(pals),
      type(t)
{
  action = g_simple_action_new("default", G_VARIANT_TYPE_STRING);
}

bool GroupInfo::addPal(PPalInfo pal) {
  if(type == GROUP_BELONG_TYPE_REGULAR) {
    LOG_WARN("should not call addPal on GROUP_BELONG_TYPE_REGULAR");
    return false;
  }
  if(hasPal(pal)) {
    return false;
  }
  members.push_back(pal);
  return true;
}

bool GroupInfo::delPal(PalInfo* pal) {
  if(type == GROUP_BELONG_TYPE_REGULAR) {
    LOG_WARN("should not call delPal on GROUP_BELONG_TYPE_REGULAR");
    return false;
  }

  for(auto it = members.begin(); it != members.end(); ++it) {
    if(it->get() == pal) {
      members.erase(it);
      return true;
    }
  }
  return false;
}

gulong GroupInfo::connect(GActionCallback c, gpointer data) {
  return g_signal_connect(this->action, "activate", GCallback(c), data);
}

void GroupInfo::activate(const string& signal) {
  auto var = g_variant_new_string(signal.c_str());
  g_action_activate(G_ACTION(action), var);
}

void GroupInfo::disconnect(gulong sigId) {
  g_signal_handler_disconnect(this->action, sigId);
}

void GroupInfo::addMsgCount(int i) {
  allMsgCount += i;
  activate("unreadMsgCountChanged");
}

int GroupInfo::getUnreadMsgCount() const {
  g_assert(allMsgCount >= readMsgCount);
  return allMsgCount - readMsgCount;
}

}
