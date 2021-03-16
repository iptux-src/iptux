//
// C++ Implementation: CoreThread
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "config.h"
#include "UiCoreThread.h"

#include <cinttypes>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include <glib/gi18n.h>
#include <glog/logging.h>

#include "iptux-core/Const.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"
#include "iptux/LogSystem.h"
#include "iptux/MainWindow.h"
#include "iptux/UiHelper.h"
#include "iptux/UiProgramData.h"

#define OCCUPY_OBJECT 0x01

using namespace std;

namespace iptux {

/**
 * 类构造函数.
 */
UiCoreThread::UiCoreThread(Application* app, shared_ptr<UiProgramData> data)
    : CoreThread(data),
      app(app),
      programData(data),
      groupInfos(NULL),
      sgmlist(NULL),
      grplist(NULL),
      brdlist(NULL),
      pbn(1),
      prn(MAX_SHAREDFILE),
      ecsList(NULL) {
  timerid = 0;
  logSystem = new LogSystem(data);
  g_queue_init(&msgline);
  InitSublayer();
}

/**
 * 类析构函数.
 */
UiCoreThread::~UiCoreThread() {
  delete logSystem;
}

/**
 * 插入消息(UI线程安全).
 * @param para 消息参数封装包
 * @note 消息数据必须使用utf8编码
 * @note (para->pal)不可为null
 * @note
 * 请不要关心函数内部实现，你只需要按照要求封装消息数据，然后扔给本函数处理就可以了，
 * 它会想办法将消息按照你所期望的格式插入到你所期望的TextBuffer，否则请发送Bug报告
 */
void UiCoreThread::InsertMessage(const MsgPara& para) {
  MsgPara para2 = para;
  this->emitEvent(make_shared<NewMessageEvent>(move(para2)));
}

void UiCoreThread::InsertMessage(MsgPara&& para) {
  this->emitEvent(make_shared<NewMessageEvent>(move(para)));
}

/**
 * 插入消息到群组消息缓冲区(非UI线程安全).
 * @param grpinf 群组信息
 * @param para 消息参数
 */
void UiCoreThread::InsertMsgToGroupInfoItem(GroupInfo *grpinf, MsgPara *para) {
  GtkTextIter iter;
  const gchar *data;

  for(size_t i = 0; i < para->dtlist.size(); ++i) {
    ChipData* chipData = &para->dtlist[i];
    data = chipData->data.c_str();
    switch (chipData->type) {
      case MESSAGE_CONTENT_TYPE_STRING:
        InsertHeaderToBuffer(grpinf->buffer, para);
        gtk_text_buffer_get_end_iter(grpinf->buffer, &iter);
        gtk_text_buffer_insert(grpinf->buffer, &iter, "\n", -1);
        InsertStringToBuffer(grpinf->buffer, data);
        gtk_text_buffer_get_end_iter(grpinf->buffer, &iter);
        gtk_text_buffer_insert(grpinf->buffer, &iter, "\n", -1);
        CommunicateLog(para, "[STRING]%s", data);
        break;
      case MESSAGE_CONTENT_TYPE_PICTURE:
        InsertPixbufToBuffer(grpinf->buffer, data);
        CommunicateLog(para, "[PICTURE]%s", data);
        break;
      default:
        break;
    }
  }
}

/**
 * 从好友链表中移除所有好友数据(非UI线程安全).
 * @note 鉴于好友链表成员不能被删除，所以将成员改为下线标记即可
 */
void UiCoreThread::ClearAllPalFromList() {
  CoreThread::ClearAllPalFromList();

  SessionAbstract *session;
  GroupInfo *grpinf;
  GSList *tlist;

  /* 清空常规模式下所有群组的成员 */
  tlist = groupInfos;
  while (tlist) {
    grpinf = (GroupInfo *)tlist->data;
    if (grpinf->dialog) {
      session = (SessionAbstract *)g_object_get_data(G_OBJECT(grpinf->dialog),
                                                     "session-class");
      session->ClearAllPalData();
    }
    tlist = g_slist_next(tlist);
  }
  /* 清空网段模式下所有群组的成员 */
  tlist = sgmlist;
  while (tlist) {
    grpinf = (GroupInfo *)tlist->data;
    if (grpinf->dialog) {
      session = (SessionAbstract *)g_object_get_data(G_OBJECT(grpinf->dialog),
                                                     "session-class");
      session->ClearAllPalData();
    }
    tlist = g_slist_next(tlist);
  }
  /* 清空分组模式下所有群组的成员 */
  tlist = grplist;
  while (tlist) {
    grpinf = (GroupInfo *)tlist->data;
    if (grpinf->dialog) {
      session = (SessionAbstract *)g_object_get_data(G_OBJECT(grpinf->dialog),
                                                     "session-class");
      session->ClearAllPalData();
    }
    tlist = g_slist_next(tlist);
  }
  /* 清空广播模式下所有群组的成员 */
  tlist = brdlist;
  while (tlist) {
    grpinf = (GroupInfo *)tlist->data;
    if (grpinf->dialog) {
      session = (SessionAbstract *)g_object_get_data(G_OBJECT(grpinf->dialog),
                                                     "session-class");
      session->ClearAllPalData();
    }
    tlist = g_slist_next(tlist);
  }
}

/**
 * 从好友链表中删除指定的好友信息数据(非UI线程安全).
 * @param ipv4 ipv4
 * @note 鉴于好友链表成员不能被删除，所以将成员改为下线标记即可；
 * 鉴于群组中只能包含在线的好友，所以若某群组中包含了此好友，则必须从此群组中删除此好友
 */
void UiCoreThread::DelPalFromList(PalKey palKey) {
  CoreThread::DelPalFromList(palKey);

  PPalInfo pal;
  GroupInfo *grpinf;

  /* 获取好友信息数据，并将其置为下线状态 */
  if (!(pal = GetPal(palKey))) return;

  /* 从群组中移除好友 */
  if ((grpinf = GetPalRegularItem(pal.get()))) {
    DelPalFromGroupInfoItem(grpinf, pal.get());
  }
  if ((grpinf = GetPalSegmentItem(pal.get()))) {
    DelPalFromGroupInfoItem(grpinf, pal.get());
  }
  if ((grpinf = GetPalGroupItem(pal.get()))) {
    DelPalFromGroupInfoItem(grpinf, pal.get());
  }
  if ((grpinf = GetPalBroadcastItem(pal.get()))) {
    DelPalFromGroupInfoItem(grpinf, pal.get());
  }
}

/**
 * 通告指定的好友信息数据已经被更新(非UI线程安全).
 * @param ipv4 ipv4
 * @note 什么时候会用到？1、好友更新个人资料；2、好友下线后又上线了
 * @note 鉴于群组中必须包含所有属于自己的成员，移除不属于自己的成员，
 * 所以好友信息更新后应该重新调整群组成员；
 * @note 群组中被更新的成员信息也应该在界面上做出相应更新
 */
void UiCoreThread::UpdatePalToList(PalKey palKey) {
  CoreThread::UpdatePalToList(palKey);

  PPalInfo ppal;
  GroupInfo *grpinf;
  SessionAbstract *session;

  /* 如果好友链表中不存在此好友，则视为程序设计出错 */
  if (!(ppal = GetPal(palKey))) {
    return;
  }

  auto pal = ppal.get();

  /* 更新好友所在的群组，以及它在UI上的信息 */
  /*/* 更新常规模式下的群组 */
  if ((grpinf = GetPalRegularItem(pal))) {
    if (!grpinf->hasPal(pal)) {
      AttachPalToGroupInfoItem(grpinf, ppal);
    } else if (grpinf->dialog) {
      session = (SessionAbstract *)g_object_get_data(G_OBJECT(grpinf->dialog),
                                                     "session-class");
      session->UpdatePalData(pal);
    }
  } else {
    if (!(grpinf = GetPalRegularItem(pal))) grpinf = AttachPalRegularItem(ppal);
    AttachPalToGroupInfoItem(grpinf, ppal);
  }
  /*/* 更新网段模式下的群组 */
  if ((grpinf = GetPalSegmentItem(pal))) {
    if (!grpinf->hasPal(pal)) {
      AttachPalToGroupInfoItem(grpinf, ppal);
    } else if (grpinf->dialog) {
      session = (SessionAbstract *)g_object_get_data(G_OBJECT(grpinf->dialog),
                                                     "session-class");
      session->UpdatePalData(pal);
    }
  } else {
    if (!(grpinf = GetPalSegmentItem(pal))) grpinf = AttachPalSegmentItem(ppal);
    AttachPalToGroupInfoItem(grpinf, ppal);
  }
  /*/* 更新分组模式下的群组 */
  if ((grpinf = GetPalPrevGroupItem(pal))) {
    if (!pal->group || strcmp(grpinf->name.c_str(), pal->group) != 0) {
      DelPalFromGroupInfoItem(grpinf, pal);
      if (!(grpinf = GetPalGroupItem(pal))) grpinf = AttachPalGroupItem(ppal);
      AttachPalToGroupInfoItem(grpinf, ppal);
    } else if (grpinf->dialog) {
      session = (SessionAbstract *)g_object_get_data(G_OBJECT(grpinf->dialog),
                                                     "session-class");
      session->UpdatePalData(pal);
    }
  } else {
    if (!(grpinf = GetPalGroupItem(pal))) grpinf = AttachPalGroupItem(ppal);
    AttachPalToGroupInfoItem(grpinf, ppal);
  }
  /*/* 更新广播模式下的群组 */
  if ((grpinf = GetPalBroadcastItem(pal))) {
    if (!grpinf->hasPal(pal)) {
      AttachPalToGroupInfoItem(grpinf, ppal);
    } else if (grpinf->dialog) {
      session = (SessionAbstract *)g_object_get_data(G_OBJECT(grpinf->dialog),
                                                     "session-class");
      session->UpdatePalData(pal);
    }
  } else {
    if (!(grpinf = GetPalBroadcastItem(pal)))
      grpinf = AttachPalBroadcastItem(ppal);
    AttachPalToGroupInfoItem(grpinf, ppal);
  }
}

/**
 * 将好友信息数据加入到好友链表(非UI线程安全).
 * @param pal class PalInfo
 * @note 鉴于在线的好友必须被分配到它所属的群组，所以加入好友到好友链表的同时
 * 也应该分配好友到相应的群组
 */
void UiCoreThread::AttachPalToList(shared_ptr<PalInfo> pal2) {
  CoreThread::AttachPalToList(pal2);
  GroupInfo *grpinf;

  auto pal = pal2.get();

  /* 将好友加入到相应的群组 */
  if (!(grpinf = GetPalRegularItem(pal))) grpinf = AttachPalRegularItem(pal2);
  AttachPalToGroupInfoItem(grpinf, pal2);
  if (!(grpinf = GetPalSegmentItem(pal))) grpinf = AttachPalSegmentItem(pal2);
  AttachPalToGroupInfoItem(grpinf, pal2);
  if (!(grpinf = GetPalGroupItem(pal))) grpinf = AttachPalGroupItem(pal2);
  AttachPalToGroupInfoItem(grpinf, pal2);
  if (!(grpinf = GetPalBroadcastItem(pal)))
    grpinf = AttachPalBroadcastItem(pal2);
  AttachPalToGroupInfoItem(grpinf, pal2);
}

/**
 * 获取(pal)在常规模式下的群组信息.
 * @param pal class PalInfo
 * @return 群组信息
 */
GroupInfo *UiCoreThread::GetPalRegularItem(PalInfo *pal) {
  GSList *tlist;

  tlist = groupInfos;
  while (tlist) {
    if (((GroupInfo *)tlist->data)->grpid == inAddrToUint32(pal->ipv4)) break;
    tlist = g_slist_next(tlist);
  }

  return (GroupInfo *)(tlist ? tlist->data : NULL);
}

/**
 * 获取(pal)在网段模式下的群组信息.
 * @param pal class PalInfo
 * @return 群组信息
 */
GroupInfo *UiCoreThread::GetPalSegmentItem(PalInfo *pal) {
  GSList *tlist;
  GQuark grpid;

  /* 获取局域网网段ID */
  auto name = ipv4_get_lan_name(pal->ipv4);
  grpid = g_quark_from_string(name.empty() ? _("Others") : name.c_str());

  tlist = sgmlist;
  while (tlist) {
    if (((GroupInfo *)tlist->data)->grpid == grpid) break;
    tlist = g_slist_next(tlist);
  }

  return (GroupInfo *)(tlist ? tlist->data : NULL);
}

/**
 * 获取(pal)在分组模式下的群组信息.
 * @param pal class PalInfo
 * @return 群组信息
 */
GroupInfo *UiCoreThread::GetPalGroupItem(PalInfo *pal) {
  GSList *tlist;
  GQuark grpid;

  /* 获取组ID */
  NO_OPERATION_C
  grpid = g_quark_from_string(pal->group ? pal->group : _("Others"));

  tlist = grplist;
  while (tlist) {
    if (((GroupInfo *)tlist->data)->grpid == grpid) break;
    tlist = g_slist_next(tlist);
  }

  return (GroupInfo *)(tlist ? tlist->data : NULL);
}

/**
 * 获取(pal)在广播模式下的群组信息.
 * @param pal class PalInfo
 * @return 群组信息
 */
GroupInfo *UiCoreThread::GetPalBroadcastItem(PalInfo *) {
  return (GroupInfo *)(brdlist ? brdlist->data : NULL);
}

/**
 * 获取消息队列项总数.
 * @return 项数
 */
guint UiCoreThread::GetMsglineItems() { return g_queue_get_length(&msgline); }

/**
 * 查看消息队列首项.
 * @return 项
 */
GroupInfo *UiCoreThread::GetMsglineHeadItem() {
  return (GroupInfo *)g_queue_peek_head(&msgline);
}

/**
 * 消息队列是否已经包含此项.
 * @param grpinf 项
 * @return 是否包含
 */
bool UiCoreThread::MsglineContainItem(GroupInfo *grpinf) {
  return g_queue_find(&msgline, grpinf);
}

/**
 * 压入项进消息队列.
 * @param grpinf 项
 */
void UiCoreThread::PushItemToMsgline(GroupInfo *grpinf) {
  g_queue_push_tail(&msgline, grpinf);
}

/**
 * 弹出项从消息队列.
 * @param grpinf 项
 */
void UiCoreThread::PopItemFromMsgline(GroupInfo *grpinf) {
  g_queue_remove(&msgline, grpinf);
}

/**
 * 初始化底层数据.
 */
void UiCoreThread::InitSublayer() {
}

/**
 * 清空底层数据.
 */
void UiCoreThread::ClearSublayer() {
  GSList *tlist;

  CoreThread::ClearSublayer();

  for (tlist = groupInfos; tlist; tlist = g_slist_next(tlist))
    delete (GroupInfo *) tlist->data;
  g_slist_free(groupInfos);
  for (tlist = sgmlist; tlist; tlist = g_slist_next(tlist))
    delete (GroupInfo *) tlist->data;
  g_slist_free(sgmlist);
  for (tlist = grplist; tlist; tlist = g_slist_next(tlist))
    delete (GroupInfo *) tlist->data;
  g_slist_free(grplist);
  for (tlist = brdlist; tlist; tlist = g_slist_next(tlist))
    delete (GroupInfo *) tlist->data;
  g_slist_free(brdlist);
  g_queue_clear(&msgline);

  for (tlist = ecsList; tlist; tlist = g_slist_next(tlist))
    delete (FileInfo *)tlist->data;
  g_slist_free(ecsList);
  if (timerid > 0) g_source_remove(timerid);
  pthread_mutex_destroy(&mutex);
}

/**
 * 插入消息头到TextBuffer(非UI线程安全).
 * @param buffer text-buffer
 * @param para 消息参数
 */
void UiCoreThread::InsertHeaderToBuffer(GtkTextBuffer *buffer, MsgPara *para) {
  GtkTextIter iter;
  gchar *header;

  auto g_progdt = getProgramData();

  /**
   * @note (para->pal)可能为null.
   */
  switch (para->stype) {
  case MessageSourceType::PAL:
      header = getformattime(FALSE, "%s",
                             para->pal ? para->pal->name : _("unknown"));
      gtk_text_buffer_get_end_iter(buffer, &iter);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, header, -1,
                                               "pal-color", NULL);
      g_free(header);
      break;
  case MessageSourceType::SELF:
      header = getformattime(FALSE, "%s", g_progdt->nickname.c_str());
      gtk_text_buffer_get_end_iter(buffer, &iter);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, header, -1,
                                               "me-color", NULL);
      g_free(header);
      break;
  case MessageSourceType::ERROR:
      header = getformattime(FALSE, "%s", _("<ERROR>"));
      gtk_text_buffer_get_end_iter(buffer, &iter);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, header, -1,
                                               "error-color", NULL);
      g_free(header);
      break;
    default:
      break;
  }
}

/**
 * 插入字符串到TextBuffer(非UI线程安全).
 * @param buffer text-buffer
 * @param string 字符串
 */
void UiCoreThread::InsertStringToBuffer(GtkTextBuffer *buffer, const gchar *string) {
  static uint32_t count = 0;
  GtkTextIter iter;
  GtkTextTag *tag;
  GMatchInfo *matchinfo;
  gchar *substring;
  char name[9];  // 8 +1  =9
  gint startp, endp;
  gint urlendp;

  auto g_progdt = getProgramData();

  urlendp = 0;
  matchinfo = NULL;
  gtk_text_buffer_get_end_iter(buffer, &iter);
  g_regex_match_full(g_progdt->urlregex, string, -1, 0, GRegexMatchFlags(0),
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
}

/**
 * 插入图片到TextBuffer(非UI线程安全).
 * @param buffer text-buffer
 * @param path 图片路径
 */
void UiCoreThread::InsertPixbufToBuffer(GtkTextBuffer *buffer, const gchar *path) {
  GtkTextIter start, end;
  GdkPixbuf *pixbuf;

  if ((pixbuf = gdk_pixbuf_new_from_file(path, NULL))) {
    gtk_text_buffer_get_start_iter(buffer, &start);
    if (gtk_text_iter_get_char(&start) == OCCUPY_OBJECT ||
        gtk_text_iter_forward_find_char(
            &start, GtkTextCharPredicate(giter_compare_foreach),
            GUINT_TO_POINTER(OCCUPY_OBJECT), NULL)) {
      end = start;
      gtk_text_iter_forward_char(&end);
      gtk_text_buffer_delete(buffer, &start, &end);
    }
    gtk_text_buffer_insert_pixbuf(buffer, &start, pixbuf);
    g_object_unref(pixbuf);
  }
}

/**
 * 获取(pal)在分组模式下当前所在的群组.
 * @param pal class PalInfo
 * @return 群组信息
 */
GroupInfo *UiCoreThread::GetPalPrevGroupItem(PalInfo *pal) {
  GSList *tlist;

  tlist = grplist;
  while (tlist) {
    if (((GroupInfo *)tlist->data)->hasPal(pal)) break;
    tlist = g_slist_next(tlist);
  }

  return (GroupInfo *)(tlist ? tlist->data : NULL);
}

/**
 * 增加新项到常规模式群组链表(非UI线程安全).
 * @param pal class PalInfo
 * @return 新加入的群组
 */
GroupInfo *UiCoreThread::AttachPalRegularItem(PPalInfo pal) {
  GroupInfo *grpinf;

  grpinf = new GroupInfo(pal);
  grpinf->grpid = inAddrToUint32(pal->ipv4);
  grpinf->name = g_strdup(pal->name);
  grpinf->buffer = gtk_text_buffer_new(programData->table);
  grpinf->dialog = NULL;
  groupInfos = g_slist_append(groupInfos, grpinf);
  return grpinf;
}

/**
 * 增加新项到网段模式群组链表(非UI线程安全).
 * @param pal class PalInfo
 * @return 新加入的群组
 */
GroupInfo *UiCoreThread::AttachPalSegmentItem(PPalInfo pal) {
  GroupInfo *grpinf;

  /* 获取局域网网段名称 */
  auto name = ipv4_get_lan_name(pal->ipv4);
  if(name.empty()) {
    name = _("Others");
  }

  grpinf = new GroupInfo(GROUP_BELONG_TYPE_SEGMENT, vector<PPalInfo>());
  grpinf->grpid = g_quark_from_static_string(name.c_str());
  grpinf->name = name;
  grpinf->buffer = gtk_text_buffer_new(programData->table);
  grpinf->dialog = NULL;
  sgmlist = g_slist_append(sgmlist, grpinf);

  return grpinf;
}

/**
 * 增加新项到分组模式群组链表(非UI线程安全).
 * @param pal class PalInfo
 * @return 新加入的群组
 */
GroupInfo *UiCoreThread::AttachPalGroupItem(PPalInfo pal) {
  GroupInfo *grpinf;
  char *name;

  /* 备份组名称，用于计算ID号 */
  NO_OPERATION_C
  name = g_strdup(pal->group ? pal->group : _("Others"));

  grpinf = new GroupInfo(GROUP_BELONG_TYPE_GROUP, vector<PPalInfo>());
  grpinf->name = name;
  grpinf->buffer = gtk_text_buffer_new(programData->table);
  grpinf->dialog = NULL;
  grplist = g_slist_append(grplist, grpinf);

  return grpinf;
}

/**
 * 增加新项到广播模式群组链表(非UI线程安全).
 * @param pal class PalInfo
 * @return 新加入的群组
 */
GroupInfo *UiCoreThread::AttachPalBroadcastItem(PPalInfo) {
  GroupInfo *grpinf;
  char *name;

  name = g_strdup(_("Broadcast"));

  grpinf = new GroupInfo(GROUP_BELONG_TYPE_BROADCAST, vector<PPalInfo>());
  grpinf->grpid = g_quark_from_static_string(name);
  grpinf->name = name;
  grpinf->buffer = gtk_text_buffer_new(programData->table);
  grpinf->dialog = NULL;
  brdlist = g_slist_append(brdlist, grpinf);

  return grpinf;
}

/**
 * 从群组中移除指定的好友(非UI线程安全).
 * @param grpinf class GroupInfo
 * @param pal class PalInfo
 */
void UiCoreThread::DelPalFromGroupInfoItem(GroupInfo *grpinf, PalInfo *pal) {
  GSList *tlist;
  SessionAbstract *session;

  grpinf->delPal(pal);
  if (grpinf->dialog) {
    session = (SessionAbstract *)g_object_get_data(G_OBJECT(grpinf->dialog),
                                                    "session-class");
    session->DelPalData(pal);
  }
}

/**
 * 添加好友到指定的群组(非UI线程安全).
 * @param grpinf class GroupInfo
 * @param pal class PalInfo
 */
void UiCoreThread::AttachPalToGroupInfoItem(GroupInfo *grpinf, PPalInfo pal) {
  SessionAbstract *session;
  grpinf->addPal(pal);
  if (grpinf->dialog) {
    session = (SessionAbstract *)g_object_get_data(G_OBJECT(grpinf->dialog),
                                                   "session-class");
    session->InsertPalData(pal.get());
  }
}

/**
 * 获取特定好友发过来的文件(非UI线程安全).
 * @param pal class PalInfo
 * @return palecslist 该好友发过来待接收的文件列表
 */
GSList *UiCoreThread::GetPalEnclosure(PalInfo *pal) {
  GSList *tlist, *palecslist;
  palecslist = NULL;
  for (tlist = ecsList; tlist; tlist = g_slist_next(tlist)) {
    if (((FileInfo *)tlist->data)->fileown->GetKey() == pal->GetKey()) {
      palecslist = g_slist_append(palecslist, tlist->data);
    }
  }
  return palecslist;
}
/**
 * 压入项进接收文件列表(非UI线程安全).
 * @param file 文件类指针
 */
void UiCoreThread::PushItemToEnclosureList(FileInfo *file) {
  ecsList = g_slist_append(ecsList, file);
  auto groupInfo = this->GetPalRegularItem(file->fileown.get());
  if(groupInfo) {
    groupInfo->activate("NEW_FILE_RECEIVED");
  }
}
/**
 * 从接收文件列表删除项(非UI线程安全).
 * @param file 文件类指针
 */
void UiCoreThread::PopItemFromEnclosureList(FileInfo *file) {
  ecsList = g_slist_remove(ecsList, file);
  delete file;
}

void UiCoreThread::start() {
  CoreThread::start();
/* 定时扫描处理程序内部任务 */
  CHECK_EQ(timerid, guint(0));
  timerid = g_timeout_add(500, GSourceFunc(WatchCoreStatus), this);
}

/**
 * 扫描处理程序内部任务(非UI线程安全).
 * @param pcthrd 核心类
 * @return GLib库所需
 */
gboolean UiCoreThread::WatchCoreStatus(UiCoreThread *pcthrd) {
  GList *tlist;

  /* 让等待队列中的群组信息项闪烁 */
  pthread_mutex_lock(&pcthrd->mutex);
  tlist = pcthrd->msgline.head;
  while (tlist) {
    pcthrd->app->getMainWindow()->MakeItemBlinking((GroupInfo *)tlist->data, true);
    tlist = g_list_next(tlist);
  }
  pthread_mutex_unlock(&pcthrd->mutex);

  return TRUE;
}

shared_ptr<UiProgramData> UiCoreThread::getUiProgramData() {
  return programData;
}

void UiCoreThread::CommunicateLog(MsgPara *msgpara, const char *fmt, ...) const {
  va_list args;
  va_start (args, fmt);
  logSystem->CommunicateLog(msgpara, fmt, args);
  va_end(args);
}

void UiCoreThread::SystemLog(const char *fmt, ...) const {
  va_list args;
  va_start (args, fmt);
  logSystem->SystemLog(fmt, args);
  va_end(args);
}

}  // namespace iptux
