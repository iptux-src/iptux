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
#include <sys/socket.h>
#include <sys/stat.h>

#include <glib/gi18n.h>
#include <glog/logging.h>

#include "iptux-core/Const.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"
#include "iptux/LogSystem.h"
#include "iptux/UiHelper.h"

using namespace std;

namespace iptux {

/**
 * 类构造函数.
 */
UiCoreThread::UiCoreThread(Application* app, shared_ptr<ProgramData> data)
    : CoreThread(data),
      programData(data),
      groupInfos(NULL),
      sgmlist(NULL),
      grplist(NULL),
      brdlist(NULL),
      pbn(1),
      prn(MAX_SHAREDFILE),
      ecsList(NULL) {
  tag_table_ = CreateTagTable();
  CheckIconTheme();
  logSystem = app->getLogSystem();
  InitSublayer();
}

/**
 * 类析构函数.
 */
UiCoreThread::~UiCoreThread() {
  g_object_unref(tag_table_);
}

/**
 * 从好友链表中移除所有好友数据(非UI线程安全).
 * @note 鉴于好友链表成员不能被删除，所以将成员改为下线标记即可
 */
void UiCoreThread::ClearAllPalFromList() {
  CoreThread::ClearAllPalFromList();

  SessionAbstract* session;
  GroupInfo* grpinf;
  GSList* tlist;

  /* 清空常规模式下所有群组的成员 */
  tlist = groupInfos;
  while (tlist) {
    grpinf = (GroupInfo*)tlist->data;
    if (grpinf->getDialog()) {
      session = (SessionAbstract*)g_object_get_data(
          G_OBJECT(grpinf->getDialog()), "session-class");
      session->ClearAllPalData();
    }
    tlist = g_slist_next(tlist);
  }
  /* 清空网段模式下所有群组的成员 */
  tlist = sgmlist;
  while (tlist) {
    grpinf = (GroupInfo*)tlist->data;
    if (grpinf->getDialog()) {
      session = (SessionAbstract*)g_object_get_data(
          G_OBJECT(grpinf->getDialog()), "session-class");
      session->ClearAllPalData();
    }
    tlist = g_slist_next(tlist);
  }
  /* 清空分组模式下所有群组的成员 */
  tlist = grplist;
  while (tlist) {
    grpinf = (GroupInfo*)tlist->data;
    if (grpinf->getDialog()) {
      session = (SessionAbstract*)g_object_get_data(
          G_OBJECT(grpinf->getDialog()), "session-class");
      session->ClearAllPalData();
    }
    tlist = g_slist_next(tlist);
  }
  /* 清空广播模式下所有群组的成员 */
  tlist = brdlist;
  while (tlist) {
    grpinf = (GroupInfo*)tlist->data;
    if (grpinf->getDialog()) {
      session = (SessionAbstract*)g_object_get_data(
          G_OBJECT(grpinf->getDialog()), "session-class");
      session->ClearAllPalData();
    }
    tlist = g_slist_next(tlist);
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
  GroupInfo* grpinf;
  SessionAbstract* session;

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
    } else if (grpinf->getDialog()) {
      session = (SessionAbstract*)g_object_get_data(
          G_OBJECT(grpinf->getDialog()), "session-class");
      session->UpdatePalData(pal);
    }
  } else {
    if (!(grpinf = GetPalRegularItem(pal)))
      grpinf = AttachPalRegularItem(ppal);
    AttachPalToGroupInfoItem(grpinf, ppal);
  }
  /*/* 更新网段模式下的群组 */
  if ((grpinf = GetPalSegmentItem(pal))) {
    if (!grpinf->hasPal(pal)) {
      AttachPalToGroupInfoItem(grpinf, ppal);
    } else if (grpinf->getDialog()) {
      session = (SessionAbstract*)g_object_get_data(
          G_OBJECT(grpinf->getDialog()), "session-class");
      session->UpdatePalData(pal);
    }
  } else {
    if (!(grpinf = GetPalSegmentItem(pal)))
      grpinf = AttachPalSegmentItem(ppal);
    AttachPalToGroupInfoItem(grpinf, ppal);
  }
  /*/* 更新分组模式下的群组 */
  if ((grpinf = GetPalPrevGroupItem(pal))) {
    if (strcmp(grpinf->name().c_str(), pal->getGroup().c_str()) != 0) {
      DelPalFromGroupInfoItem(grpinf, pal);
      if (!(grpinf = GetPalGroupItem(pal)))
        grpinf = AttachPalGroupItem(ppal);
      AttachPalToGroupInfoItem(grpinf, ppal);
    } else if (grpinf->getDialog()) {
      session = (SessionAbstract*)g_object_get_data(
          G_OBJECT(grpinf->getDialog()), "session-class");
      session->UpdatePalData(pal);
    }
  } else {
    if (!(grpinf = GetPalGroupItem(pal)))
      grpinf = AttachPalGroupItem(ppal);
    AttachPalToGroupInfoItem(grpinf, ppal);
  }
  /*/* 更新广播模式下的群组 */
  if ((grpinf = GetPalBroadcastItem(pal))) {
    if (!grpinf->hasPal(pal)) {
      AttachPalToGroupInfoItem(grpinf, ppal);
    } else if (grpinf->getDialog()) {
      session = (SessionAbstract*)g_object_get_data(
          G_OBJECT(grpinf->getDialog()), "session-class");
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
  GroupInfo* grpinf;

  auto pal = pal2.get();

  /* 将好友加入到相应的群组 */
  if (!(grpinf = GetPalRegularItem(pal)))
    grpinf = AttachPalRegularItem(pal2);
  if (!(grpinf = GetPalSegmentItem(pal)))
    grpinf = AttachPalSegmentItem(pal2);
  AttachPalToGroupInfoItem(grpinf, pal2);
  if (!(grpinf = GetPalGroupItem(pal)))
    grpinf = AttachPalGroupItem(pal2);
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
GroupInfo* UiCoreThread::GetPalRegularItem(const PalInfo* pal) {
  GSList* tlist;

  tlist = groupInfos;
  while (tlist) {
    if (((GroupInfo*)tlist->data)->grpid == inAddrToUint32(pal->ipv4()))
      break;
    tlist = g_slist_next(tlist);
  }

  return (GroupInfo*)(tlist ? tlist->data : NULL);
}

/**
 * 获取(pal)在网段模式下的群组信息.
 * @param pal class PalInfo
 * @return 群组信息
 */
GroupInfo* UiCoreThread::GetPalSegmentItem(const PalInfo* pal) {
  GSList* tlist;
  GQuark grpid;

  /* 获取局域网网段ID */
  auto name = ipv4_get_lan_name(pal->ipv4());
  grpid = g_quark_from_string(name.empty() ? _("Others") : name.c_str());

  tlist = sgmlist;
  while (tlist) {
    if (((GroupInfo*)tlist->data)->grpid == grpid)
      break;
    tlist = g_slist_next(tlist);
  }

  return (GroupInfo*)(tlist ? tlist->data : NULL);
}

/**
 * 获取(pal)在分组模式下的群组信息.
 * @param pal class PalInfo
 * @return 群组信息
 */
GroupInfo* UiCoreThread::GetPalGroupItem(const PalInfo* pal) {
  GSList* tlist;
  GQuark grpid;

  /* 获取组ID */
  NO_OPERATION_C
  auto group = pal->getGroup();
  grpid = g_quark_from_string(group.empty() ? _("Others") : group.c_str());

  tlist = grplist;
  while (tlist) {
    if (((GroupInfo*)tlist->data)->grpid == grpid)
      break;
    tlist = g_slist_next(tlist);
  }

  return (GroupInfo*)(tlist ? tlist->data : NULL);
}

/**
 * 获取(pal)在广播模式下的群组信息.
 * @param pal class PalInfo
 * @return 群组信息
 */
GroupInfo* UiCoreThread::GetPalBroadcastItem(const PalInfo*) {
  return (GroupInfo*)(brdlist ? brdlist->data : NULL);
}

/**
 * 初始化底层数据.
 */
void UiCoreThread::InitSublayer() {}

/**
 * 清空底层数据.
 */
void UiCoreThread::ClearSublayer() {
  GSList* tlist;

  CoreThread::ClearSublayer();

  for (tlist = groupInfos; tlist; tlist = g_slist_next(tlist))
    delete (GroupInfo*)tlist->data;
  g_slist_free(groupInfos);
  for (tlist = sgmlist; tlist; tlist = g_slist_next(tlist))
    delete (GroupInfo*)tlist->data;
  g_slist_free(sgmlist);
  for (tlist = grplist; tlist; tlist = g_slist_next(tlist))
    delete (GroupInfo*)tlist->data;
  g_slist_free(grplist);
  for (tlist = brdlist; tlist; tlist = g_slist_next(tlist))
    delete (GroupInfo*)tlist->data;
  g_slist_free(brdlist);

  for (tlist = ecsList; tlist; tlist = g_slist_next(tlist))
    delete (FileInfo*)tlist->data;
  g_slist_free(ecsList);
}

/**
 * 获取(pal)在分组模式下当前所在的群组.
 * @param pal class PalInfo
 * @return 群组信息
 */
GroupInfo* UiCoreThread::GetPalPrevGroupItem(PalInfo* pal) {
  GSList* tlist;

  tlist = grplist;
  while (tlist) {
    if (((GroupInfo*)tlist->data)->hasPal(pal))
      break;
    tlist = g_slist_next(tlist);
  }

  return (GroupInfo*)(tlist ? tlist->data : NULL);
}

/**
 * 增加新项到常规模式群组链表(非UI线程安全).
 * @param pal class PalInfo
 * @return 新加入的群组
 */
GroupInfo* UiCoreThread::AttachPalRegularItem(PPalInfo pal) {
  GroupInfo* grpinf;

  grpinf = new GroupInfo(pal, getMe(), logSystem);
  grpinf->grpid = inAddrToUint32(pal->ipv4());
  grpinf->buffer = gtk_text_buffer_new(tag_table_);
  grpinf->clearDialog();
  grpinf->signalUnreadMsgCountUpdated.connect(
      sigc::mem_fun(*this, &UiCoreThread::onGroupInfoMsgCountUpdate));
  groupInfos = g_slist_append(groupInfos, grpinf);
  return grpinf;
}

/**
 * 增加新项到网段模式群组链表(非UI线程安全).
 * @param pal class PalInfo
 * @return 新加入的群组
 */
GroupInfo* UiCoreThread::AttachPalSegmentItem(PPalInfo pal) {
  GroupInfo* grpinf;

  /* 获取局域网网段名称 */
  auto name = ipv4_get_lan_name(pal->ipv4());
  if (name.empty()) {
    name = _("Others");
  }

  grpinf = new GroupInfo(IPTUX_GROUP_BELONG_SEGMENT, vector<PPalInfo>(),
                         getMe(), name, logSystem);
  grpinf->grpid = g_quark_from_static_string(name.c_str());
  grpinf->buffer = gtk_text_buffer_new(tag_table_);
  grpinf->clearDialog();
  sgmlist = g_slist_append(sgmlist, grpinf);

  return grpinf;
}

/**
 * 增加新项到分组模式群组链表(非UI线程安全).
 * @param pal class PalInfo
 * @return 新加入的群组
 */
GroupInfo* UiCoreThread::AttachPalGroupItem(PPalInfo pal) {
  GroupInfo* grpinf;
  auto name = pal->getGroup();
  if (name.empty()) {
    name = _("Others");
  }
  grpinf = new GroupInfo(IPTUX_GROUP_BELONG_GROUP, vector<PPalInfo>(), getMe(),
                         name, logSystem);
  grpinf->buffer = gtk_text_buffer_new(tag_table_);
  grpinf->clearDialog();
  grplist = g_slist_append(grplist, grpinf);

  return grpinf;
}

/**
 * 增加新项到广播模式群组链表(非UI线程安全).
 * @param pal class PalInfo
 * @return 新加入的群组
 */
GroupInfo* UiCoreThread::AttachPalBroadcastItem(PPalInfo) {
  GroupInfo* grpinf;
  char* name;

  name = g_strdup(_("Broadcast"));

  grpinf = new GroupInfo(IPTUX_GROUP_BELONG_BROADCAST, vector<PPalInfo>(),
                         getMe(), name, logSystem);
  grpinf->grpid = g_quark_from_static_string(name);
  grpinf->buffer = gtk_text_buffer_new(tag_table_);
  grpinf->clearDialog();
  brdlist = g_slist_append(brdlist, grpinf);

  return grpinf;
}

/**
 * 从群组中移除指定的好友(非UI线程安全).
 * @param grpinf class GroupInfo
 * @param pal class PalInfo
 */
void UiCoreThread::DelPalFromGroupInfoItem(GroupInfo* grpinf, PalInfo* pal) {
  SessionAbstract* session;

  grpinf->delPal(pal);
  if (grpinf->getDialog()) {
    session = (SessionAbstract*)g_object_get_data(G_OBJECT(grpinf->getDialog()),
                                                  "session-class");
    session->DelPalData(pal);
  }
}

/**
 * 添加好友到指定的群组(非UI线程安全).
 * @param grpinf class GroupInfo
 * @param pal class PalInfo
 */
void UiCoreThread::AttachPalToGroupInfoItem(GroupInfo* grpinf, PPalInfo pal) {
  SessionAbstract* session;
  grpinf->addPal(pal);
  if (grpinf->getDialog()) {
    session = (SessionAbstract*)g_object_get_data(G_OBJECT(grpinf->getDialog()),
                                                  "session-class");
    session->InsertPalData(pal.get());
  }
}

/**
 * 获取特定好友发过来的文件(非UI线程安全).
 * @param pal class PalInfo
 * @return palecslist 该好友发过来待接收的文件列表
 */
GSList* UiCoreThread::GetPalEnclosure(PalInfo* pal) {
  GSList *tlist, *palecslist;
  palecslist = NULL;
  for (tlist = ecsList; tlist; tlist = g_slist_next(tlist)) {
    if (((FileInfo*)tlist->data)->fileown->GetKey() == pal->GetKey()) {
      palecslist = g_slist_append(palecslist, tlist->data);
    }
  }
  return palecslist;
}
/**
 * 压入项进接收文件列表(非UI线程安全).
 * @param file 文件类指针
 */
void UiCoreThread::PushItemToEnclosureList(FileInfo* file) {
  ecsList = g_slist_append(ecsList, file);
  auto groupInfo = this->GetPalRegularItem(file->fileown.get());
  if (groupInfo) {
    groupInfo->newFileReceived();
  }
}
/**
 * 从接收文件列表删除项(非UI线程安全).
 * @param file 文件类指针
 */
void UiCoreThread::PopItemFromEnclosureList(FileInfo* file) {
  ecsList = g_slist_remove(ecsList, file);
  delete file;
}

void UiCoreThread::onGroupInfoMsgCountUpdate(GroupInfo* grpinf, int, int) {
  sigGroupInfoUpdated.emit(grpinf);
  sigUnreadMsgCountUpdated.emit(unread_msg_count());
}

/**
 * 创建用于(text-view)的一些通用tag.
 * @note 给这些tag一个"global"标记，表示这些对象是全局共享的
 */
GtkTextTagTable* UiCoreThread::CreateTagTable() {
  GtkTextTag* tag;

  GtkTextTagTable* table = gtk_text_tag_table_new();

  tag = gtk_text_tag_new("pal-color");
  g_object_set(tag, "foreground", "blue", NULL);
  g_object_set_data(G_OBJECT(tag), "global", GINT_TO_POINTER(TRUE));
  gtk_text_tag_table_add(table, tag);
  g_object_unref(tag);

  tag = gtk_text_tag_new("me-color");
  g_object_set(tag, "foreground", "green", NULL);
  g_object_set_data(G_OBJECT(tag), "global", GINT_TO_POINTER(TRUE));
  gtk_text_tag_table_add(table, tag);
  g_object_unref(tag);

  tag = gtk_text_tag_new("error-color");
  g_object_set(tag, "foreground", "red", NULL);
  g_object_set_data(G_OBJECT(tag), "global", GINT_TO_POINTER(TRUE));
  gtk_text_tag_table_add(table, tag);
  g_object_unref(tag);

  tag = gtk_text_tag_new("sign-words");
  g_object_set(tag, "indent", 10, "foreground", "#1005F0", "font",
               "Sans Italic 8", NULL);
  g_object_set_data(G_OBJECT(tag), "global", GINT_TO_POINTER(TRUE));
  gtk_text_tag_table_add(table, tag);
  g_object_unref(tag);

  tag = gtk_text_tag_new("url-link");
  g_object_set(tag, "foreground", "blue", "underline", PANGO_UNDERLINE_SINGLE,
               NULL);
  g_object_set_data(G_OBJECT(tag), "global", GINT_TO_POINTER(TRUE));
  gtk_text_tag_table_add(table, tag);
  g_object_unref(tag);
  return table;
}

/**
 * 确保头像数据被存放在主题库中.
 */
void UiCoreThread::CheckIconTheme() {
  char pathbuf[MAX_PATHLEN];
  GdkPixbuf* pixbuf;

  snprintf(pathbuf, MAX_PATHLEN, __PIXMAPS_PATH "/icon/%s",
           programData->myicon.c_str());
  if (access(pathbuf, F_OK) != 0) {
    snprintf(pathbuf, MAX_PATHLEN, "%s" ICON_PATH "/%s",
             g_get_user_config_dir(), programData->myicon.c_str());
    if ((pixbuf = gdk_pixbuf_new_from_file(pathbuf, NULL))) {
      gtk_icon_theme_add_builtin_icon(programData->myicon.c_str(), MAX_ICONSIZE,
                                      pixbuf);
      g_object_unref(pixbuf);
    }
  }

  snprintf(pathbuf, MAX_PATHLEN, __PIXMAPS_PATH "/icon/%s",
           programData->palicon);
  if (access(pathbuf, F_OK) != 0) {
    snprintf(pathbuf, MAX_PATHLEN, "%s" ICON_PATH "/%s",
             g_get_user_config_dir(), programData->palicon);
    if ((pixbuf = gdk_pixbuf_new_from_file(pathbuf, NULL))) {
      gtk_icon_theme_add_builtin_icon(programData->palicon, MAX_ICONSIZE,
                                      pixbuf);
      g_object_unref(pixbuf);
    }
  }
}

int UiCoreThread::unread_msg_count() const {
  int count = 0;
  GSList* tlist;

  for (tlist = groupInfos; tlist; tlist = g_slist_next(tlist)) {
    count += ((GroupInfo*)tlist->data)->getUnreadMsgCount();
  }
  return count;
}

}  // namespace iptux
