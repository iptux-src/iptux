//
// C++ Interface: CoreThread
//
// Description:
// 程序中的核心线程类，实际上也被设计成了所有底层核心数据的中心点，
// 所有数据的更新、查询、插入、删除都必须通过本类接口才能完成。
// -----------------------------------------------------
// 2012.02:把文件传送的核心数据全部放在CoreThread类。
// prlist不变,增加ecsList来存放好友发来文件.
//------------------------------------------------------
// Author: cwll <cwll2009@126.com>, (C) 2012
//         Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_UICORETHREAD_H
#define IPTUX_UICORETHREAD_H

#include <queue>

#include "iptux/UiProgramData.h"
#include "iptux-core/Models.h"
#include "iptux/UiModels.h"
#include "iptux-core/CoreThread.h"

namespace iptux {

class LogSystem;

/**
 * @note 请保证插入或更新某成员时，底层优先于UI；删除某成员时，UI优先于底层，
 * 否则你会把所有事情都搞砸. \n
 * @note 鉴于(GroupInfo::member)成员发生变动时必须保证函数处于UI线程安全的环境，
 * 所以UI线程安全的函数对(GroupInfo::member)的访问无须加锁.\n
 * 若此特性不可被如此利用，请报告bug. \n
 * @note 如果本程序编码中的某处没有遵循以上规则，请报告bug.
 */
class UiCoreThread: public CoreThread {
 public:
  explicit UiCoreThread(std::shared_ptr<UiProgramData> data);
  ~UiCoreThread() override ;

  std::shared_ptr<UiProgramData> getUiProgramData();

  void start() override;

  void InsertMessage(const MsgPara& para);
  void InsertMessage(MsgPara&& para);

  void InsertMsgToGroupInfoItem(GroupInfo *grpinf, MsgPara *para);

  void ClearAllPalFromList() override ;
  void DelPalFromList(PalKey palKey) override ;
  void UpdatePalToList(PalKey palKey) override ;
  void AttachPalToList(PalInfo *pal) override ;
  void AttachPalToList(std::shared_ptr<PalInfo> pal) override ;
  GroupInfo *GetPalRegularItem(PalInfo *pal);
  GroupInfo *GetPalSegmentItem(PalInfo *pal);
  GroupInfo *GetPalGroupItem(PalInfo *pal);
  GroupInfo *GetPalBroadcastItem(PalInfo *pal);

  guint GetMsglineItems();
  GroupInfo *GetMsglineHeadItem();
  bool MsglineContainItem(GroupInfo *grpinf);
  void PushItemToMsgline(GroupInfo *grpinf);
  void PopItemFromMsgline(GroupInfo *grpinf);

  GSList *GetPalEnclosure(PalInfo *pal);
  void PushItemToEnclosureList(FileInfo *file);
  void PopItemFromEnclosureList(FileInfo *file);

  void CommunicateLog(MsgPara *msgpara, const char *fmt, ...) const G_GNUC_PRINTF(3, 4);
  void SystemLog(const char *fmt, ...) const G_GNUC_PRINTF(2, 3);

 private:
  void InitSublayer();
  void ClearSublayer() override ;

  static void InsertHeaderToBuffer(GtkTextBuffer *buffer, MsgPara *para);
  static void InsertStringToBuffer(GtkTextBuffer *buffer, const gchar *string);
  static void InsertPixbufToBuffer(GtkTextBuffer *buffer, const gchar *path);

  GroupInfo *GetPalPrevGroupItem(PalInfo *pal);
  GroupInfo *AttachPalRegularItem(PalInfo *pal);
  GroupInfo *AttachPalSegmentItem(PalInfo *pal);
  GroupInfo *AttachPalGroupItem(PalInfo *pal);
  GroupInfo *AttachPalBroadcastItem(PalInfo *pal);
  static void DelPalFromGroupInfoItem(GroupInfo *grpinf, PalInfo *pal);
  static void AttachPalToGroupInfoItem(GroupInfo *grpinf, PalInfo *pal);

private:
  std::shared_ptr<UiProgramData> programData;
  LogSystem* logSystem;
  guint timerid;          //定时器ID
  std::queue<MsgPara> messages;

  GSList *groupInfos, *sgmlist, *grplist, *brdlist;  //群组链表(成员不能被删除)
  GQueue msgline;                                 //消息队列

  uint32_t pbn, prn;        //当前已使用的文件编号(共享/私有)
  GSList *ecsList;          //文件链表(好友发过来)
  //        GSList *rcvdList;               //文件链表(好友发过来已接收)

  //回调处理部分函数
 private:
  static gboolean WatchCoreStatus(UiCoreThread *self);

    //内联成员函数
 public:
  inline uint32_t &PbnQuote() { return pbn; }

  inline uint32_t &PrnQuote() { return prn; }
};

}  // namespace iptux

#endif
