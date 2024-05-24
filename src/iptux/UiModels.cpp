#include "config.h"
#include "UiModels.h"

#include <cstring>
#include <glib/gi18n.h>
#include <glog/logging.h>

#include "iptux-core/Const.h"
#include "iptux-core/Models.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"
#include "iptux/DialogBase.h"
#include "iptux/UiHelper.h"
#include <netinet/in.h>

using namespace std;

namespace iptux {

const char* const kObjectKeyImagePath = "image-path";

/**
 * 文件传输树(trans-tree)底层数据结构.
 * 14,0 status,1 task,2 peer,3 ip,4 filename,5 filelength,6 finishlength,7
 * progress, 8 pro-text,9 cost,10 remain,11 rate,12,pathname,13 data,14 para, 15
 * finished
 *
 * 任务状态;任务类型;任务对端;文件名(如果当前是文件夹，该项指正在传输的文件夹内单个文件,
 * 整个文件夹传输完成后,该项指向当前是文件夹);文件长度;完成长度;完成进度;
 * 进度串;已花费时间;任务剩余时间;传输速度;带路径文件名(不显示);文件传输类;参数指针值
 *
 * @return trans-model
 */
TransModel* transModelNew() {
  GtkListStore* model;
  model = gtk_list_store_new(int(TransModelColumn::N_COLUMNS),
                             G_TYPE_STRING,   // STATUS
                             G_TYPE_STRING,   // TASK
                             G_TYPE_STRING,   // PEER
                             G_TYPE_STRING,   // IP
                             G_TYPE_STRING,   // FILENAME
                             G_TYPE_STRING,   // FILE_LENGTH_TEXT
                             G_TYPE_STRING,   // FINISHED_LENGTH_TEXT
                             G_TYPE_INT,      // PROGRESS
                             G_TYPE_STRING,   // PROGRESS_TEXT
                             G_TYPE_STRING,   // COST
                             G_TYPE_STRING,   // REMAIN
                             G_TYPE_STRING,   // RATE
                             G_TYPE_STRING,   // FILE_PATH
                             G_TYPE_BOOLEAN,  // FINISHED
                             G_TYPE_INT       // TASK_ID
  );
  return GTK_TREE_MODEL(model);
}

void transModelDelete(TransModel* model) {
  g_object_unref(model);
}

static void transModelFillFromTransFileModel(TransModel* model,
                                             GtkTreeIter* iter,
                                             const TransFileModel& para) {
  gtk_list_store_set(
      GTK_LIST_STORE(model), iter,                                           //
      TransModelColumn::STATUS, para.getStatus().c_str(),                    //
      TransModelColumn::TASK, para.getTask().c_str(),                        //
      TransModelColumn::PEER, para.getPeer().c_str(),                        //
      TransModelColumn::IP, para.getIp().c_str(),                            //
      TransModelColumn::FILENAME, para.getFilename().c_str(),                //
      TransModelColumn::FILE_LENGTH_TEXT, para.getFileLengthText().c_str(),  //
      TransModelColumn::FINISHED_LENGTH_TEXT,
      para.getFinishedLengthText().c_str(),                             //
      TransModelColumn::PROGRESS, int(para.getProgress()),              //
      TransModelColumn::PROGRESS_TEXT, para.getProgressText().c_str(),  //
      TransModelColumn::COST, para.getCost().c_str(),                   //
      TransModelColumn::REMAIN, para.getRemain().c_str(),               //
      TransModelColumn::RATE, para.getRate().c_str(),                   //
      TransModelColumn::FILE_PATH, para.getFilePath().c_str(),          //
      TransModelColumn::FINISHED, para.isFinished(),                    //
      TransModelColumn::TASK_ID, para.getTaskId(),                      //
      -1);
}

void transModelUpdateFromTransFileModel(TransModel* model,
                                        const TransFileModel& transFileModel) {
  GtkTreeIter iter;
  bool found = false;
  if (gtk_tree_model_get_iter_first(model, &iter)) {
    do {
      int taskId = 0;
      gtk_tree_model_get(model, &iter, TransModelColumn::TASK_ID, &taskId, -1);
      if (taskId == transFileModel.getTaskId()) {
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

void transModelLoadFromTransFileModels(
    TransModel* model,
    const vector<unique_ptr<TransFileModel>>& fileModels) {
  gtk_list_store_clear(GTK_LIST_STORE(model));
  for (auto& it : fileModels) {
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
gint paltreeCompareByNameFunc(GtkTreeModel* model,
                              GtkTreeIter* a,
                              GtkTreeIter* b) {
  GroupInfo *agrpinf, *bgrpinf;
  gint result;

  gtk_tree_model_get(model, a, PalTreeModelColumn::DATA, &agrpinf, -1);
  gtk_tree_model_get(model, b, PalTreeModelColumn::DATA, &bgrpinf, -1);
  result = strcmp(agrpinf->name().c_str(), bgrpinf->name().c_str());

  return result;
}

gint paltreeCompareByUserNameFunc(GtkTreeModel* model,
                                  GtkTreeIter* a,
                                  GtkTreeIter* b) {
  GroupInfo *agrpinf, *bgrpinf;
  gint result;

  gtk_tree_model_get(model, a, PalTreeModelColumn::DATA, &agrpinf, -1);
  gtk_tree_model_get(model, b, PalTreeModelColumn::DATA, &bgrpinf, -1);
  result = strcmp(agrpinf->user_name().c_str(), bgrpinf->user_name().c_str());

  return result;
}

/**
 * 好友树(paltree)按IP排序的比较函数.
 * @param model paltree-model
 * @param a A GtkTreeIter in model
 * @param b Another GtkTreeIter in model
 * @return 比较值
 */
gint paltreeCompareByIPFunc(GtkTreeModel* model,
                            GtkTreeIter* a,
                            GtkTreeIter* b) {
  GroupInfo *agrpinf, *bgrpinf;
  gtk_tree_model_get(model, a, PalTreeModelColumn::DATA, &agrpinf, -1);
  gtk_tree_model_get(model, b, PalTreeModelColumn::DATA, &bgrpinf, -1);
  if (agrpinf->getType() == GROUP_BELONG_TYPE_REGULAR &&
      bgrpinf->getType() == GROUP_BELONG_TYPE_REGULAR) {
    if (agrpinf->grpid < bgrpinf->grpid) {
      return -1;
    }
    if (agrpinf->grpid == bgrpinf->grpid) {
      return 0;
    }
    return 1;
  }

  return 0;
}

gint paltreeCompareByHostFunc(GtkTreeModel* model,
                              GtkTreeIter* a,
                              GtkTreeIter* b) {
  GroupInfo *agrpinf, *bgrpinf;

  gtk_tree_model_get(model, a, PalTreeModelColumn::DATA, &agrpinf, -1);
  gtk_tree_model_get(model, b, PalTreeModelColumn::DATA, &bgrpinf, -1);
  return strcmp(agrpinf->host().c_str(), bgrpinf->host().c_str());
}

gint paltreeCompareByLastActivityFunc(GtkTreeModel* model,
                                      GtkTreeIter* a,
                                      GtkTreeIter* b) {
  GroupInfo *agrpinf, *bgrpinf;

  gtk_tree_model_get(model, a, PalTreeModelColumn::DATA, &agrpinf, -1);
  gtk_tree_model_get(model, b, PalTreeModelColumn::DATA, &bgrpinf, -1);
  if (agrpinf->last_activity() < bgrpinf->last_activity()) {
    return -1;
  } else if (agrpinf->last_activity() == bgrpinf->last_activity()) {
    return 0;
  } else {
    return 1;
  }
}

/**
 * 好友树(paltree)底层数据结构.
 * 7,0 closed-expander,1 open-expander,2 info.,3 extras,4 style,5 color,6 data
 * \n 关闭的展开器;打开的展开器;群组信息;扩展信息;字体风格;字体颜色;群组数据 \n
 * @return paltree-model
 */
PalTreeModel* palTreeModelNew() {
  return palTreeModelNew(PalTreeModelSortKey::IP, GTK_SORT_ASCENDING);
}

PalTreeModel* palTreeModelNew(PalTreeModelSortKey sort_key,
                              GtkSortType sort_type) {
  GtkTreeStore* model;

  model =
      gtk_tree_store_new(int(PalTreeModelColumn::N_COLUMNS), GDK_TYPE_PIXBUF,
                         GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING,
                         PANGO_TYPE_ATTR_LIST, GDK_TYPE_RGBA, G_TYPE_POINTER);
  palTreeModelSetSortKey(GTK_TREE_MODEL(model), sort_key);
  gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model),
                                       GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
                                       sort_type);

  return GTK_TREE_MODEL(model);
}

void palTreeModelSetSortKey(PalTreeModel* model, PalTreeModelSortKey key) {
  GtkTreeIterCompareFunc f = PalTreeModelSortKeyToCompareFunc(key);
  if (!f) {
    LOG_WARN("unknown PalTreeModelSortKey: %d", int(key));
    return;
  }
  gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(model), f, NULL,
                                          NULL);
}

GroupInfo* PalTreeModelGetGroupInfo(PalTreeModel* model, GtkTreeIter* iter) {
  GroupInfo* pgrpinf;
  gtk_tree_model_get(model, iter, PalTreeModelColumn::DATA, &pgrpinf, -1);
  return pgrpinf;
}

static const GdkRGBA color = {0.3216, 0.7216, 0.2196, 0.0};

/**
 * 填充群组数据(grpinf)到数据集(model)指定位置(iter).
 * @param model model
 * @param iter iter
 * @param grpinf class GroupInfo
 */
void palTreeModelFillFromGroupInfo(GtkTreeModel* model,
                                   GtkTreeIter* iter,
                                   const GroupInfo* grpinf,
                                   GroupInfoStyle style,
                                   const string& font) {
  GtkIconTheme* theme;
  GdkPixbuf *cpixbuf, *opixbuf = nullptr;
  PangoAttrList* attrs;
  PangoAttribute* attr;
  string extra;
  PalInfo* pal;
  GError* error = nullptr;

  /* 创建图标 */
  theme = gtk_icon_theme_get_default();
  if (grpinf->getType() == GROUP_BELONG_TYPE_REGULAR) {
    pal = grpinf->getMembers()[0].get();
    auto file = iptux_erase_filename_suffix(pal->icon_file().c_str());
    cpixbuf = gtk_icon_theme_load_icon(theme, file, MAX_ICONSIZE,
                                       GtkIconLookupFlags(0), &error);
    if (cpixbuf == nullptr) {
      LOG_WARN("gtk_icon_theme_load_icon failed: [%d] %s", error->code,
               error->message);
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

  /* 创建扩展信息 */
  if (grpinf->getType() != GROUP_BELONG_TYPE_REGULAR) {
    extra = stringFormat("(%d)", (int)(grpinf->getMembers().size()));
  }

  /* 创建字体风格 */
  attrs = pango_attr_list_new();
  if (grpinf->getType() == GROUP_BELONG_TYPE_REGULAR) {
    auto dspt = pango_font_description_from_string(font.c_str());
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
  gtk_tree_store_set(
      GTK_TREE_STORE(model), iter, PalTreeModelColumn ::CLOSED_EXPANDER,
      cpixbuf, PalTreeModelColumn ::OPEN_EXPANDER, opixbuf,
      PalTreeModelColumn ::INFO, grpinf->GetInfoAsMarkup(style).c_str(),
      PalTreeModelColumn ::EXTRAS, extra.c_str(), PalTreeModelColumn ::STYLE,
      attrs, PalTreeModelColumn ::COLOR, &color, PalTreeModelColumn ::DATA,
      grpinf, -1);

  /* 释放资源 */
  if (cpixbuf)
    g_object_unref(cpixbuf);
  if (opixbuf)
    g_object_unref(opixbuf);
  pango_attr_list_unref(attrs);
}

static const char* group_info_style_names[] = {
    [(int)GroupInfoStyle::IP] = "ip",
    [(int)GroupInfoStyle::HOST] = "host",
    [(int)GroupInfoStyle::USERNAME] = "username",
    [(int)GroupInfoStyle::VERSION_NAME] = "version",
    [(int)GroupInfoStyle::LAST_ACTIVITY] = "last_activity",
    [(int)GroupInfoStyle::LAST_MESSAGE] = "last_message",
    [(int)GroupInfoStyle::IP_PORT] = "ip_port",
};

GroupInfoStyle GroupInfoStyleFromStr(const std::string& s) {
  for (int i = 0; i < (int)GroupInfoStyle::INVALID; ++i) {
    if (s == group_info_style_names[i]) {
      return (GroupInfoStyle)i;
    }
  }
  return GroupInfoStyle::INVALID;
}

const char* GroupInfoStyleToStr(GroupInfoStyle style) {
  if (style >= GroupInfoStyle::IP && style < GroupInfoStyle::INVALID) {
    return group_info_style_names[(int)style];
  }
  return "";
}

static const char* gtk_sort_type_names[] = {
    [GTK_SORT_ASCENDING] = "ascending",
    [GTK_SORT_DESCENDING] = "descending",
};

GtkSortType GtkSortTypeFromStr(const std::string& s) {
  for (int i = GTK_SORT_ASCENDING; i <= GTK_SORT_DESCENDING; ++i) {
    if (s == gtk_sort_type_names[i]) {
      return (GtkSortType)i;
    }
  }
  return GTK_SORT_TYPE_INVALID;
}
const char* GtkSortTypeToStr(GtkSortType t) {
  if (GTK_SORT_ASCENDING <= t && t <= GTK_SORT_DESCENDING) {
    return gtk_sort_type_names[t];
  }
  return "";
}

// GroupInfo::GroupInfo()
//     : grpid(0),
//       type(GROUP_BELONG_TYPE_REGULAR),
//       member(NULL),
//       buffer(NULL),
//       dialog(NULL) {}
GroupInfo::~GroupInfo() {
  g_object_unref(buffer);
}

bool GroupInfo::hasPal(PalInfo* pal) const {
  for (auto i : members) {
    if (i.get() == pal) {
      return true;
    }
  }
  return false;
}

bool GroupInfo::hasPal(PPalInfo pal) const {
  return hasPal(pal.get());
}

string GroupInfo::user_name() const {
  if (getType() == GROUP_BELONG_TYPE_REGULAR) {
    auto pal = this->getMembers()[0].get();
    return pal->getUser();
  }
  return "";
}

GroupInfo::GroupInfo(PPalInfo pal, CPPalInfo me, LogSystem* logSystem)
    : grpid(0),
      buffer(NULL),
      dialogBase(NULL),
      me(me),
      type(GROUP_BELONG_TYPE_REGULAR),
      logSystem(logSystem) {
  members.push_back(pal);
  inputBuffer = gtk_text_buffer_new(NULL);
  name_ = pal->getName();
  host_ = pal->getHost();
}

GroupInfo::GroupInfo(iptux::GroupBelongType t,
                     const vector<PPalInfo>& pals,
                     CPPalInfo me,
                     const string& name,
                     LogSystem* logSystem)
    : grpid(0),
      buffer(NULL),
      dialogBase(NULL),
      me(me),
      members(pals),
      type(t),
      logSystem(logSystem) {
  inputBuffer = gtk_text_buffer_new(NULL);
  name_ = name;
}

GtkWidget* GroupInfo::getDialog() const {
  return dialogBase ? GTK_WIDGET(dialogBase->getWindow()) : nullptr;
}

bool GroupInfo::addPal(PPalInfo pal) {
  if (type == GROUP_BELONG_TYPE_REGULAR) {
    LOG_WARN("should not call addPal on GROUP_BELONG_TYPE_REGULAR");
    return false;
  }
  if (hasPal(pal)) {
    return false;
  }
  members.push_back(pal);
  return true;
}

bool GroupInfo::delPal(PalInfo* pal) {
  if (type == GROUP_BELONG_TYPE_REGULAR) {
    LOG_WARN("should not call delPal on GROUP_BELONG_TYPE_REGULAR");
    return false;
  }

  for (auto it = members.begin(); it != members.end(); ++it) {
    if (it->get() == pal) {
      members.erase(it);
      return true;
    }
  }
  return false;
}

void GroupInfo::newFileReceived() {
  this->signalNewFileReceived.emit(this);
}

void GroupInfo::addMsgCount(int i) {
  int oldCount = getUnreadMsgCount();
  allMsgCount += i;
  signalUnreadMsgCountUpdated.emit(this, oldCount, getUnreadMsgCount());
}

void GroupInfo::readAllMsg() {
  int oldCount = getUnreadMsgCount();
  if (oldCount != 0) {
    readMsgCount = allMsgCount;
    signalUnreadMsgCountUpdated.emit(this, oldCount, getUnreadMsgCount());
  }
}

int GroupInfo::getUnreadMsgCount() const {
  g_assert(allMsgCount >= readMsgCount);
  return allMsgCount - readMsgCount;
}

string GroupInfo::GetInfoAsMarkup(GroupInfoStyle style) const {
  string info;
  /* 创建主信息 */
  if (getType() == GROUP_BELONG_TYPE_REGULAR) {
    string line2;
    auto pal = this->getMembers()[0].get();

    switch (style) {
      case GroupInfoStyle::HOST:
        line2 = this->host();
        break;
      case GroupInfoStyle::VERSION_NAME:
        line2 = pal->getVersion();
        break;
      case GroupInfoStyle::USERNAME:
        line2 = user_name();
        break;
      case GroupInfoStyle::LAST_ACTIVITY:
        line2 = last_activity_ ? TimeToStr(last_activity_) : "";
        break;
      case GroupInfoStyle::LAST_MESSAGE:
        line2 = last_message_;
        break;
      case GroupInfoStyle::IP_PORT:
        line2 = stringFormat("%s:%d", inAddrToString(pal->ipv4()).c_str(),
                             pal->port());
        break;
      case GroupInfoStyle::IP:
      default:
        auto pal = this->getMembers()[0].get();
        line2 = inAddrToString(pal->ipv4());
    }

    int unreadMsgCount = this->getUnreadMsgCount();
    if (unreadMsgCount > 0) {
      return stringFormat("%s <span foreground=\"red\">(%d)</span>\n%s",
                          markupEscapeText(pal->getName()).c_str(),
                          unreadMsgCount, markupEscapeText(line2).c_str());
    } else {
      return stringFormat("%s\n%s", markupEscapeText(pal->getName()).c_str(),
                          markupEscapeText(line2).c_str());
    }
  } else {
    return markupEscapeText(this->name());
  }
}

string GroupInfo::GetHintAsMarkup() const {
  if (this->type != GROUP_BELONG_TYPE_REGULAR) {
    return "";
  }
  auto pal = this->members[0];

  ostringstream res;
  res << MarkupPrintf(_("Version: %s"), pal->getVersion().c_str());
  res << "\n";

  if (!pal->getGroup().empty()) {
    res << MarkupPrintf(_("Nickname: %s@%s"), pal->getName().c_str(),
                        pal->getGroup().c_str());
  } else {
    res << MarkupPrintf(_("Nickname: %s"), pal->getName().c_str());
  }
  res << "\n";

  res << MarkupPrintf(_("User: %s"), pal->getUser().c_str());
  res << "\n";

  res << MarkupPrintf(_("Host: %s"), pal->getHost().c_str());
  res << "\n";

  string ipstr = inAddrToString(pal->ipv4());
  if (pal->segdes && *pal->segdes != '\0') {
    res << MarkupPrintf(_("Address: %s(%s)"), pal->segdes, ipstr.c_str());
  } else {
    res << MarkupPrintf(_("Address: %s"), ipstr.c_str());
  }
  res << "\n";

  if (!pal->isCompatible()) {
    res << markupEscapeText(_("Compatibility: Microsoft"));
  } else {
    res << markupEscapeText(_("Compatibility: GNU/Linux"));
  }
  res << "\n";

  res << MarkupPrintf(_("System coding: %s"), pal->getEncode().c_str());

  if (pal->sign && pal->sign[0]) {
    string signature1;
    string signature2;
    signature1 = markupEscapeText(_("Signature:"));
    signature2 = markupEscapeText(pal->sign);
    res << stringFormat(
        "\n%s\n<span foreground=\"#00FF00\" "
        "font_style=\"italic\" size=\"smaller\">%s</span>",
        signature1.c_str(), signature2.c_str());
  }

  return res.str();
}

/**
 * 插入字符串到TextBuffer(非UI线程安全).
 * @param buffer text-buffer
 * @param string 字符串
 */
static void InsertStringToBuffer(GtkTextBuffer* buffer, const gchar* s) {
  static uint32_t count = 0;
  GtkTextIter iter;
  GtkTextTag* tag;
  GMatchInfo* matchinfo;
  gchar* substring;
  char name[9];  // 8 +1  =9
  gint startp, endp;
  gint urlendp;

  auto s2 = utf8MakeValid(s);
  auto string = s2.c_str();

  urlendp = 0;
  matchinfo = NULL;
  gtk_text_buffer_get_end_iter(buffer, &iter);
  g_regex_match_full(getUrlRegex(), string, -1, 0, GRegexMatchFlags(0),
                     &matchinfo, NULL);
  while (g_match_info_matches(matchinfo)) {
    snprintf(name, 9, "%" PRIx32, count++);
    tag = gtk_text_buffer_create_tag(buffer, name, NULL);
    substring = g_match_info_fetch(matchinfo, 0);
    g_object_set_data_full(G_OBJECT(tag), "url", substring,
                           GDestroyNotify(g_free));
    g_match_info_fetch_pos(matchinfo, 0, &startp, &endp);
    gtk_text_buffer_insert(buffer, &iter, string + urlendp, startp - urlendp);
    gtk_text_buffer_insert_with_tags_by_name(
        buffer, &iter, string + startp, endp - startp, "url-link", name, NULL);
    urlendp = endp;
    g_match_info_next(matchinfo, NULL);
  }
  g_match_info_free(matchinfo);
  gtk_text_buffer_insert(buffer, &iter, string + urlendp, -1);
  gtk_text_buffer_get_end_iter(buffer, &iter);
  gtk_text_buffer_insert(buffer, &iter, "\n", -1);
}

/**
 * 插入消息头到TextBuffer(非UI线程安全).
 * @param buffer text-buffer
 * @param para 消息参数
 */
static void InsertHeaderToBuffer(GtkTextBuffer* buffer,
                                 const MsgPara* para,
                                 CPPalInfo me,
                                 time_t now) {
  GtkTextIter iter;
  gchar* header;

  /**
   * @note (para->pal)可能为null.
   */
  switch (para->stype) {
    case MessageSourceType::PAL:
      header =
          getformattime2(now, FALSE, "%s", para->getPal()->getName().c_str());
      gtk_text_buffer_get_end_iter(buffer, &iter);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, header, -1,
                                               "pal-color", NULL);
      g_free(header);
      break;
    case MessageSourceType::SELF:
      header = getformattime2(now, FALSE, "%s", me->getName().c_str());
      gtk_text_buffer_get_end_iter(buffer, &iter);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, header, -1,
                                               "me-color", NULL);
      g_free(header);
      break;
    case MessageSourceType::ERROR:
      header = getformattime2(now, FALSE, "%s", _("<ERROR>"));
      gtk_text_buffer_get_end_iter(buffer, &iter);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, header, -1,
                                               "error-color", NULL);
      g_free(header);
      break;
    default:
      break;
  }
  gtk_text_buffer_get_end_iter(buffer, &iter);
  gtk_text_buffer_insert(buffer, &iter, "\n", -1);
}

/**
 * 插入图片到TextBuffer.
 * @param buffer text-buffer
 * @param path 图片路径
 */
static void InsertPixbufToBuffer(GtkTextBuffer* buffer, const gchar* path) {
  GtkTextIter iter;

  gtk_text_buffer_get_end_iter(buffer, &iter);
  GtkTextChildAnchor* anchor = gtk_text_child_anchor_new();
  g_object_set_data_full(G_OBJECT(anchor), kObjectKeyImagePath, g_strdup(path),
                         GDestroyNotify(g_free));
  gtk_text_buffer_insert_child_anchor(buffer, &iter, anchor);
  gtk_text_buffer_get_end_iter(buffer, &iter);
  gtk_text_buffer_insert(buffer, &iter, "\n", -1);
}

void GroupInfo::addMsgPara(const MsgPara& para) {
  time_t now = time(NULL);
  _addMsgPara(para, now);
}

void GroupInfo::_addMsgPara(const MsgPara& para, time_t now) {
  const gchar* data;

  time(&last_activity_);

  for (size_t i = 0; i < para.dtlist.size(); ++i) {
    const ChipData* chipData = &para.dtlist[i];
    data = chipData->data.c_str();
    switch (chipData->type) {
      case MESSAGE_CONTENT_TYPE_STRING:
        InsertHeaderToBuffer(buffer, &para, me, now);
        InsertStringToBuffer(buffer, data);
        last_message_ = StrFirstNonEmptyLine(chipData->data);
        if (logSystem) {
          logSystem->communicateLog(&para, "[STRING]%s", data);
        }
        break;
      case MESSAGE_CONTENT_TYPE_PICTURE:
        InsertHeaderToBuffer(buffer, &para, me, now);
        InsertPixbufToBuffer(buffer, data);
        last_message_ = _("[IMG]");
        if (logSystem) {
          logSystem->communicateLog(&para, "[PICTURE]%s", data);
        }
        break;
      default:
        break;
    }
  }
  addMsgCount(1);
}

bool transModelIsFinished(TransModel* model) {
  GtkTreeIter iter;
  if (gtk_tree_model_get_iter_first(model, &iter)) {
    do {
      gboolean finished = false;
      gtk_tree_model_get(model, &iter, TransModelColumn::FINISHED, &finished,
                         -1);

      if (!finished)
        return false;
    } while (gtk_tree_model_iter_next(model, &iter));
  }
  return true;
}

IconModel* iconModelNew() {
  return gtk_list_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_STRING);
}

const char* pal_tree_model_sort_key_names[] = {
    [(int)PalTreeModelSortKey::NICKNAME] = "nickname",
    [(int)PalTreeModelSortKey::USERNAME] = "username",
    [(int)PalTreeModelSortKey::IP] = "ip",
    [(int)PalTreeModelSortKey::HOST] = "host",
    [(int)PalTreeModelSortKey::LAST_ACTIVITY] = "last_activity",
};

PalTreeModelSortKey PalTreeModelSortKeyFromStr(const std::string& s) {
  for (int i = 0; i < (int)PalTreeModelSortKey::INVALID; ++i) {
    if (s == pal_tree_model_sort_key_names[i]) {
      return (PalTreeModelSortKey)i;
    }
  }
  return PalTreeModelSortKey::INVALID;
}

const char* PalTreeModelSortKeyToStr(PalTreeModelSortKey k) {
  if (PalTreeModelSortKey::NICKNAME <= k && k < PalTreeModelSortKey::INVALID) {
    return pal_tree_model_sort_key_names[(int)k];
  }
  return "";
}

GtkTreeIterCompareFunc PalTreeModelSortKeyToCompareFunc(PalTreeModelSortKey k) {
  switch (k) {
    case PalTreeModelSortKey::NICKNAME:
      return GtkTreeIterCompareFunc(paltreeCompareByNameFunc);
    case PalTreeModelSortKey::USERNAME:
      return GtkTreeIterCompareFunc(paltreeCompareByNameFunc);
    case PalTreeModelSortKey::IP:
      return GtkTreeIterCompareFunc(paltreeCompareByIPFunc);
    case PalTreeModelSortKey::HOST:
      return GtkTreeIterCompareFunc(paltreeCompareByHostFunc);
    case PalTreeModelSortKey::LAST_ACTIVITY:
      return GtkTreeIterCompareFunc(paltreeCompareByLastActivityFunc);
    default:
      return nullptr;
  }
}

}  // namespace iptux
