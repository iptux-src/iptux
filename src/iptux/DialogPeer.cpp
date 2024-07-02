//
// C++ Implementation: DialogPeer
//
// Description:
//
//
// Author: cwll <cwll2009@126.com> ,(C) 2012.02
//        Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "config.h"
#include "DialogPeer.h"

#include "UiModels.h"
#include "iptux-core/Models.h"
#include <cinttypes>

#include <sys/socket.h>
#include <unistd.h>

#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>
#include <glog/logging.h>

#include "iptux-core/Const.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"
#include "iptux/UiCoreThread.h"
#include "iptux/UiHelper.h"
#include "iptux/callback.h"
#include "iptux/dialog.h"

using namespace std;

namespace iptux {

/**
 * 类构造函数.
 * @param grp 好友群组信息
 */
DialogPeer::DialogPeer(Application* app, GroupInfo* grp)
    : DialogBase(CHECK_NOTNULL(app), CHECK_NOTNULL(grp)),
      config(app->getConfig()),
      torcvsize(0),
      rcvdsize(0),
      timerrcv(0) {
  ReadUILayout();
  grp->signalNewFileReceived.connect(
      sigc::mem_fun(*this, &DialogPeer::onNewFileReceived));
  app->getCoreThread()->sigGroupInfoUpdated.connect(
      sigc::mem_fun(*this, &DialogPeer::onGroupInfoUpdated));
  init();
}

/**
 * 类析构函数.
 */
DialogPeer::~DialogPeer() {
  g_signal_handler_disconnect(G_OBJECT(grpinf->getInputBuffer()), sigId);
  /* 非常重要，必须在窗口析构之前把定时触发事件停止，不然会出现意想不到的情况 */
  if (timerrcv > 0)
    g_source_remove(timerrcv);
  WriteUILayout();
}

/**
 * 好友对话框入口.
 * @param grpinf 好友群组信息
 */
void DialogPeer::PeerDialogEntry(Application* app, GroupInfo* grpinf) {
  if (grpinf->getDialog())
    return;

  new DialogPeer(app, grpinf);
}

void DialogPeer::init() {
  auto dlgpr = this;
  auto window = GTK_WIDGET(dlgpr->CreateMainWindow());
  grpinf->setDialogBase(this);
  CreateTitle();
  gtk_container_add(GTK_CONTAINER(window), dlgpr->CreateAllArea());
  gtk_widget_show_all(window);
  gtk_widget_grab_focus(GTK_WIDGET(inputTextviewWidget));
  GActionEntry win_entries[] = {
      makeActionEntry("attach_file", G_ACTION_CALLBACK(onAttachFile)),
      makeActionEntry("attach_folder", G_ACTION_CALLBACK(onAttachFolder)),
      makeActionEntry("clear_chat_history",
                      G_ACTION_CALLBACK(onClearChatHistory)),
      makeActionEntry("refuse", G_ACTION_CALLBACK(onRefuse)),
      makeActionEntry("refuse_all", G_ACTION_CALLBACK(onRefuseAll)),
      makeActionEntry("insert_picture", G_ACTION_CALLBACK(onInsertPicture)),
      makeActionEntry("request_shared_resources",
                      G_ACTION_CALLBACK(onRequestSharedResources)),
      makeActionEntry("send_message", G_ACTION_CALLBACK(onSendMessage)),
      makeActionEntry("paste", G_ACTION_CALLBACK(onPaste)),
      makeActionEntry("save_image", G_ACTION_CALLBACK(onSaveImage)),
      makeActionEntry("copy_image", G_ACTION_CALLBACK(onCopyImage)),
  };
  g_action_map_add_action_entries(G_ACTION_MAP(window), win_entries,
                                  G_N_ELEMENTS(win_entries), this);
  g_action_map_disable_actions(G_ACTION_MAP(window), "refuse", nullptr);

  sigId = g_signal_connect(G_OBJECT(getInputBuffer()), "changed",
                           G_CALLBACK(onInputBufferChanged), this);
  g_signal_connect_swapped(G_OBJECT(fileSendModel), "row-deleted",
                           G_CALLBACK(onSendFileModelChanged), this);
  g_signal_connect_swapped(G_OBJECT(fileSendModel), "row-inserted",
                           G_CALLBACK(onSendFileModelChanged), this);
  refreshSendAction();
}

/**
 * 更新好友信息.
 * @param pal 好友信息
 */
void DialogPeer::UpdatePalData(PalInfo* pal) {
  GtkWidget* textview;
  GtkTextBuffer* buffer;
  GtkTextIter start, end;

  textview = GTK_WIDGET(g_datalist_get_data(&widset, "info-textview-widget"));
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
  gtk_text_buffer_get_bounds(buffer, &start, &end);
  gtk_text_buffer_delete(buffer, &start, &end);
  FillPalInfoToBuffer(buffer, pal);
  refreshTitle();
}

string DialogPeer::GetTitle() {
  auto palinfor = grpinf->getMembers()[0].get();
  return stringFormat(_("Talk with %s(%s) IP:%s"), palinfor->getName().c_str(),
                      palinfor->getHost().c_str(),
                      inAddrToString(palinfor->ipv4()).c_str());
}

void DialogPeer::refreshTitle() {
  gtk_window_set_title(GTK_WINDOW(window), GetTitle().c_str());
}

/**
 * 插入好友数据.
 * @param pal 好友信息
 */
void DialogPeer::InsertPalData(PalInfo*) {
  // 此函数暂且无须实现
}

/**
 * 删除好友数据.
 * @param pal 好友信息
 */
void DialogPeer::DelPalData(PalInfo*) {
  // 此函数暂且无须实现
}

/**
 * 清除本群组所有好友数据.
 */
void DialogPeer::ClearAllPalData() {
  // 此函数暂且无须实现
}

/**
 * 读取对话框的UI布局数据.
 */
void DialogPeer::ReadUILayout() {
  g_datalist_set_data(
      &dtset, "window-width",
      GINT_TO_POINTER(config->GetInt("peer_window_width", 570)));
  g_datalist_set_data(
      &dtset, "window-height",
      GINT_TO_POINTER(config->GetInt("peer_window_height", 420)));
  g_datalist_set_data(
      &dtset, "main-paned-divide",
      GINT_TO_POINTER(config->GetInt("peer_main_paned_divide", 375)));
  g_datalist_set_data(
      &dtset, "historyinput-paned-divide",
      GINT_TO_POINTER(config->GetInt("peer_historyinput_paned_divide", 255)));
  g_datalist_set_data(
      &dtset, "infoenclosure-paned-divide",
      GINT_TO_POINTER(config->GetInt("peer_infoenclosure_paned_divide", 255)));
  g_datalist_set_data(
      &dtset, "enclosure-paned-divide",
      GINT_TO_POINTER(config->GetInt("peer_enclosure_paned_divide", 280)));
  g_datalist_set_data(
      &dtset, "file-receive-paned-divide",
      GINT_TO_POINTER(config->GetInt("peer_file_recieve_paned_divide", 140)));
}

/**
 * 保存对话框的UI布局数据.
 */
void DialogPeer::WriteUILayout() {
  config->SetInt("peer_window_width",
                 GPOINTER_TO_INT(g_datalist_get_data(&dtset, "window-width")));
  config->SetInt("peer_window_height",
                 GPOINTER_TO_INT(g_datalist_get_data(&dtset, "window-height")));
  config->SetInt("peer_main_paned_divide", GPOINTER_TO_INT(g_datalist_get_data(
                                               &dtset, "main-paned-divide")));
  config->SetInt("peer_historyinput_paned_divide",
                 GPOINTER_TO_INT(
                     g_datalist_get_data(&dtset, "historyinput-paned-divide")));
  config->SetInt("peer_infoenclosure_paned_divide",
                 GPOINTER_TO_INT(g_datalist_get_data(
                     &dtset, "infoenclosure-paned-divide")));
  config->SetInt(
      "peer_enclosure_paned_divide",
      GPOINTER_TO_INT(g_datalist_get_data(&dtset, "enclosure-paned-divide")));
  config->SetInt("peer_file_recieve_paned_divide",
                 GPOINTER_TO_INT(
                     g_datalist_get_data(&dtset, "file-receive-paned-divide")));
  config->Save();
}

/**
 * 创建主窗口.
 * @return 窗口
 */
GtkWindow* DialogPeer::CreateMainWindow() {
  gint width, height;

  window = GTK_APPLICATION_WINDOW(gtk_application_window_new(app->getApp()));
  refreshTitle();
  width = GPOINTER_TO_INT(g_datalist_get_data(&dtset, "window-width"));
  height = GPOINTER_TO_INT(g_datalist_get_data(&dtset, "window-height"));
  gtk_window_set_default_size(GTK_WINDOW(window), width, height);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  widget_enable_dnd_uri(GTK_WIDGET(window));
  g_datalist_set_data(&widset, "window-widget", window);
  g_object_set_data(G_OBJECT(window), "dialog", this);

  MainWindowSignalSetup(GTK_WINDOW(window));
  g_signal_connect_swapped(GTK_WIDGET(window), "show",
                           G_CALLBACK(ShowDialogPeer), this);
  afterWindowCreated();
  return GTK_WINDOW(window);
}

void DialogPeer::CreateTitle() {
  if (app->use_header_bar()) {
    GtkHeaderBar* headerBar = CreateHeaderBar(GTK_WINDOW(window), app->menu());
    gtk_header_bar_set_title(headerBar, GetTitle().c_str());
  }
}

/**
 * 创建所有区域.
 * @return 主窗体
 */
GtkWidget* DialogPeer::CreateAllArea() {
  GtkWidget* box;
  GtkWidget *hpaned, *vpaned;
  gint position;

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  /* 加入主区域 */
  hpaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
  g_datalist_set_data(&widset, "main-paned", hpaned);
  g_object_set_data(G_OBJECT(hpaned), "position-name",
                    (gpointer) "main-paned-divide");
  position = GPOINTER_TO_INT(g_datalist_get_data(&dtset, "main-paned-divide"));
  gtk_paned_set_position(GTK_PANED(hpaned), position);
  gtk_box_pack_start(GTK_BOX(box), hpaned, TRUE, TRUE, 0);
  g_signal_connect(hpaned, "notify::position", G_CALLBACK(PanedDivideChanged),
                   &dtset);
  /* 加入聊天历史记录&输入区域 */
  vpaned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
  g_object_set_data(G_OBJECT(vpaned), "position-name",
                    (gpointer) "historyinput-paned-divide");
  position =
      GPOINTER_TO_INT(g_datalist_get_data(&dtset, "historyinput-paned-divide"));
  gtk_paned_set_position(GTK_PANED(vpaned), position);
  gtk_paned_pack1(GTK_PANED(hpaned), vpaned, TRUE, TRUE);
  g_signal_connect(vpaned, "notify::position", G_CALLBACK(PanedDivideChanged),
                   &dtset);
  gtk_paned_pack1(GTK_PANED(vpaned), CreateHistoryArea(), TRUE, TRUE);
  gtk_paned_pack2(GTK_PANED(vpaned), CreateInputArea(), FALSE, TRUE);
  /* 加入好友信息&附件区域 */
  vpaned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
  g_object_set_data(G_OBJECT(vpaned), "position-name",
                    (gpointer) "infoenclosure-paned-divide");
  position = GPOINTER_TO_INT(
      g_datalist_get_data(&dtset, "infoenclosure-paned-divide"));
  gtk_paned_set_position(GTK_PANED(vpaned), position);
  gtk_paned_pack2(GTK_PANED(hpaned), vpaned, FALSE, TRUE);
  g_signal_connect(vpaned, "notify::position", G_CALLBACK(PanedDivideChanged),
                   &dtset);
  gtk_paned_pack1(GTK_PANED(vpaned), CreateInfoArea(), TRUE, TRUE);
  gtk_paned_pack2(GTK_PANED(vpaned), CreateFileArea(), FALSE, TRUE);

  return box;
}

/**
 * 创建好友信息区域.
 * @return 主窗体
 */
GtkWidget* DialogPeer::CreateInfoArea() {
  GtkWidget *frame, *sw;
  GtkWidget* widget;
  GtkTextBuffer* buffer;

  frame = gtk_frame_new(_("Info."));
  g_datalist_set_data(&widset, "info-frame", frame);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
  sw = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                      GTK_SHADOW_ETCHED_IN);
  gtk_container_add(GTK_CONTAINER(frame), sw);

  buffer = gtk_text_buffer_new(app->getCoreThread()->tag_table());
  FillPalInfoToBuffer(buffer, grpinf->getMembers()[0].get());
  widget = gtk_text_view_new_with_buffer(buffer);
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(widget), FALSE);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(widget), FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(widget), GTK_WRAP_NONE);
  gtk_container_add(GTK_CONTAINER(sw), widget);
  g_datalist_set_data(&widset, "info-textview-widget", widget);

  return frame;
}

/**
 * 将好友信息数据写入指定的缓冲区.
 * @param buffer text-buffer
 * @param pal class PalInfo
 */
void DialogPeer::FillPalInfoToBuffer(GtkTextBuffer* buffer, PalInfo* pal) {
  GdkPixbuf* pixbuf;
  GtkTextIter iter;
  string buf;

  buf += stringFormat(_("Version: %s\n"), pal->getVersion().c_str());
  if (!pal->getGroup().empty())
    buf += stringFormat(_("Nickname: %s@%s\n"), pal->getName().c_str(),
                        pal->getGroup().c_str());
  else
    buf += stringFormat(_("Nickname: %s\n"), pal->getName().c_str());

  buf += stringFormat(_("User: %s\n"), pal->getUser().c_str());
  buf += stringFormat(_("Host: %s\n"), pal->getHost().c_str());
  string ipstr = inAddrToString(pal->ipv4());
  if (pal->segdes && *pal->segdes != '\0')
    buf += stringFormat(_("Address: %s(%s)\n"), pal->segdes, ipstr.c_str());
  else
    buf += stringFormat(_("Address: %s\n"), ipstr.c_str());

  if (!pal->isCompatible()) {
    buf += _("Compatibility: Microsoft\n");
  } else {
    buf += _("Compatibility: GNU/Linux\n");
  }
  buf += stringFormat(_("System coding: %s\n"), pal->getEncode().c_str());
  gtk_text_buffer_set_text(buffer, buf.c_str(), -1);

  gtk_text_buffer_get_end_iter(buffer, &iter);
  if (pal->sign && *pal->sign != '\0') {
    gtk_text_buffer_insert(buffer, &iter, _("Signature:\n"), -1);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, pal->sign, -1,
                                             "sign-words", NULL);
  }

  if (pal->photo && *pal->photo != '\0' &&
      (pixbuf = gdk_pixbuf_new_from_file(pal->photo, NULL))) {
    gtk_text_buffer_insert(buffer, &iter, _("\nPhoto:\n"), -1);
    // TODO 缩放多少才合适
    pixbuf_shrink_scale_1(&pixbuf, 200, -1);
    gtk_text_buffer_insert_pixbuf(buffer, &iter, pixbuf);
    g_object_unref(pixbuf);
  }
}

/**
 * 发送附件给好友
 */
void DialogPeer::BroadcastEnclosureMsg(const vector<FileInfo*>& files) {
  vector<const PalInfo*> pals;
  pals.push_back(grpinf->getMembers()[0].get());
  this->app->getCoreThread()->BcstFileInfoEntry(pals, files);
}

/**
 * 发送文本消息.
 * @return 是否发送数据
 */
bool DialogPeer::SendTextMsg() {
  gtk_widget_grab_focus(GTK_WIDGET(inputTextviewWidget));  // 为下一次任务做准备
  if (grpinf->isInputEmpty()) {
    return false;
  }
  shared_ptr<MsgPara> para = grpinf->genMsgParaFromInput();
  grpinf->clearInputBuffer();

  /* 清空缓冲区并发送数据 */
  FeedbackMsg(para);
  app->getCoreThread()->AsyncSendMsgPara(para);
  return true;
}

/**
 * 回馈消息.
 * @param dtlist 数据链表
 * @note 请不要修改链表(dtlist)中的数据
 */
void DialogPeer::FeedbackMsg(shared_ptr<MsgPara> msgPara) {
  MsgPara para(grpinf->getMembers()[0]);

  para.stype = MessageSourceType::SELF;
  para.btype = grpinf->getType();
  para.dtlist = msgPara->dtlist;

  grpinf->addMsgPara(para);
}

/**
 * 封装消息.
 * @param dtlist 数据链表
 * @return 消息封装包
 */
MsgPara* DialogPeer::PackageMsg(const std::vector<ChipData>& dtlist) {
  MsgPara* para;
  auto g_cthrd = app->getCoreThread();

  para = new MsgPara(grpinf->getMembers()[0]);
  para->stype = MessageSourceType::SELF;
  para->btype = grpinf->getType();
  para->dtlist = dtlist;

  return para;
}

/**
 * 请求获取此好友的共享文件.
 * @param grpinf 好友群组信息
 */
void DialogPeer::onRequestSharedResources(void*, void*, DialogPeer& self) {
  auto g_cthrd = self.app->getCoreThread();
  g_cthrd->SendAskShared(self.grpinf->getMembers()[0]);
}

void DialogPeer::onPaste(void*, void*, DialogPeer* self) {
  GtkTextView* textview = GTK_TEXT_VIEW(self->inputTextviewWidget);
  g_signal_emit_by_name(textview, "paste-clipboard");
}

void DialogPeer::insertPicture() {
  GtkWidget* widget;
  GtkTextBuffer* buffer;
  GtkTextIter iter;
  GdkPixbuf* pixbuf;
  gchar* filename;
  gint position;
  GError* error = nullptr;

  if (!(filename = choose_file_with_preview(
            _("Please select a picture to insert the buffer"),
            GTK_WIDGET(window))))
    return;

  if (!(pixbuf = gdk_pixbuf_new_from_file(filename, &error))) {
    LOG_WARN("failed to load image: %s", error->message);
    g_error_free(error);
    error = nullptr;
    g_free(filename);
    return;
  }
  g_free(filename);

  widget = GTK_WIDGET(g_datalist_get_data(&widset, "input-textview-widget"));
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
  g_object_get(buffer, "cursor-position", &position, NULL);
  gtk_text_buffer_get_iter_at_offset(buffer, &iter, position);
  gtk_text_buffer_insert_pixbuf(buffer, &iter, pixbuf);
  g_object_unref(pixbuf);
}

/**
 * 创建文件接收和发送区域.
 * @return 主窗体
 */
GtkWidget* DialogPeer::CreateFileArea() {
  GtkWidget *frame, *vpaned;
  gint position;

  frame = gtk_frame_new(_("Enclosure."));
  g_datalist_set_data(&widset, "file-enclosure-frame-widget", frame);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
  vpaned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
  g_object_set_data(G_OBJECT(vpaned), "position-name",
                    (gpointer) "enclosure-paned-divide");
  position =
      GPOINTER_TO_INT(g_datalist_get_data(&dtset, "enclosure-paned-divide"));
  gtk_paned_set_position(GTK_PANED(vpaned), position);
  g_signal_connect(vpaned, "notify::position", G_CALLBACK(PanedDivideChanged),
                   &dtset);
  gtk_container_add(GTK_CONTAINER(frame), vpaned);
  gtk_paned_pack1(GTK_PANED(vpaned), CreateFileReceiveArea(), TRUE, TRUE);
  gtk_paned_pack2(GTK_PANED(vpaned), CreateFileSendArea(), FALSE, TRUE);
  return frame;
}

/**
 * 创建文件接收区域.
 * @return 主窗体
 */
GtkWidget* DialogPeer::CreateFileReceiveArea() {
  GtkWidget* vpaned;
  gint position;
  vpaned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
  g_datalist_set_data(&widset, "file-receive-paned-widget", vpaned);
  g_object_set_data(G_OBJECT(vpaned), "position-name",
                    (gpointer) "file-receive-paned-divide");
  position =
      GPOINTER_TO_INT(g_datalist_get_data(&dtset, "file-receive-paned-divide"));
  gtk_paned_set_position(GTK_PANED(vpaned), position);
  g_signal_connect(vpaned, "notify::position", G_CALLBACK(PanedDivideChanged),
                   &dtset);
  gtk_paned_pack1(GTK_PANED(vpaned), CreateFileToReceiveArea(), TRUE, FALSE);
  gtk_paned_pack2(GTK_PANED(vpaned), CreateFileReceivedArea(), TRUE, FALSE);
  return vpaned;
}
/**
 * 创建待接收文件区域.
 * @return 主窗体
 */
GtkWidget* DialogPeer::CreateFileToReceiveArea() {
  GtkWidget *frame, *hbox, *vbox, *button, *pbar, *sw, *treeview;
  GtkTreeModel* model;

  frame = gtk_frame_new(_("Files to be received"));
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
  pbar = gtk_progress_bar_new();
  g_datalist_set_data(&widset, "file-receive-progress-bar-widget", pbar);
  gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbar), _("Receiving progress."));
  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
  gtk_box_pack_start(GTK_BOX(hbox), pbar, TRUE, TRUE, 0);
  button = gtk_button_new_with_label(_("Accept"));
  g_signal_connect_swapped(button, "clicked", G_CALLBACK(onAcceptButtonClicked),
                           this);
  g_datalist_set_data(&widset, "file-receive-accept-button", button);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 0);

  button = gtk_button_new_with_label(_("Refuse"));
  gtk_actionable_set_action_name(GTK_ACTIONABLE(button), "win.refuse");
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 0);

  g_datalist_set_data(&widset, "file-receive-refuse-button", button);
  button = gtk_button_new_with_label(_("Detail"));
  gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, TRUE, 0);
  gtk_actionable_set_action_name(GTK_ACTIONABLE(button),
                                 "app.tools.transmission");

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
  sw = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                      GTK_SHADOW_ETCHED_IN);
  model = CreateFileToReceiveModel();
  g_datalist_set_data_full(&mdlset, "file-to-receive-model", model,
                           GDestroyNotify(g_object_unref));
  treeview = CreateFileToReceiveTree(model);
  this->fileToReceiveTreeviewWidget = treeview;
  g_object_set_data(G_OBJECT(treeview), "dialog", this);
  gtk_container_add(GTK_CONTAINER(sw), treeview);
  gtk_box_pack_end(GTK_BOX(vbox), sw, TRUE, TRUE, 0);
  gtk_container_add(GTK_CONTAINER(frame), vbox);

  return frame;
}
/**
 * 创建已接收文件区域.
 * @return 主窗体
 */
GtkWidget* DialogPeer::CreateFileReceivedArea() {
  GtkWidget *frame, *sw, *treeview;
  GtkTreeModel* model;
  frame = gtk_frame_new(_("File received."));
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
  sw = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                      GTK_SHADOW_ETCHED_IN);
  model = CreateFileReceivedModel();
  g_datalist_set_data_full(&mdlset, "file-received-model", model,
                           GDestroyNotify(g_object_unref));
  treeview = CreateFileReceivedTree(model);
  g_datalist_set_data(&widset, "file-received-treeview-widget", treeview);
  g_object_set_data(G_OBJECT(treeview), "dialog", this);
  gtk_container_add(GTK_CONTAINER(sw), treeview);
  gtk_container_add(GTK_CONTAINER(frame), sw);

  return frame;
}

/**
 * 创建待接收文件树(FileToReceive-tree).
 * @param model FileToReceive-model
 * @return 待接收文件树
 */
GtkWidget* DialogPeer::CreateFileToReceiveTree(GtkTreeModel* model) {
  GtkWidget* view;
  GtkTreeSelection* selection;
  GtkCellRenderer* cell;
  GtkTreeViewColumn* column;

  view = gtk_tree_view_new_with_model(model);
  this->fileToReceiveTree = GTK_TREE_VIEW(view);
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), TRUE);
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
  gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

  cell = gtk_cell_renderer_pixbuf_new();
  column =
      gtk_tree_view_column_new_with_attributes("", cell, "icon-name", 0, NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Source"), cell, "text",
                                                    1, NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_text_new();
  g_object_set(cell, "editable", TRUE, NULL);
  column = gtk_tree_view_column_new_with_attributes(_("SaveAs"), cell, "text",
                                                    2, NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Size"), cell, "text", 3,
                                                    NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  g_signal_connect(GTK_WIDGET(view), "button_press_event",
                   G_CALLBACK(RcvTreePopup), this);

  g_signal_connect_swapped(selection, "changed",
                           G_CALLBACK(onRecvTreeSelectionChanged), this);

  return view;
}

void DialogPeer::onRecvTreeSelectionChanged(DialogPeer& self,
                                            GtkTreeSelection* selection) {
  gint count = gtk_tree_selection_count_selected_rows(selection);
  if (count > 0) {
    g_action_map_enable_actions(G_ACTION_MAP(self.window), "refuse", nullptr);
  } else {
    g_action_map_disable_actions(G_ACTION_MAP(self.window), "refuse", nullptr);
  }
}
/**
 * 创建待接收文件树底层数据结构.
 * @return FileToReceive-model
 */
GtkTreeModel* DialogPeer::CreateFileToReceiveModel() {
  GtkListStore* model;

  model = gtk_list_store_new(6, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                             G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);

  return GTK_TREE_MODEL(model);
}
/**
 * 创建已接收文件树(FileReceived-tree).
 * @param model FileReceived-model
 * @return 已接收文件树
 */
GtkWidget* DialogPeer::CreateFileReceivedTree(GtkTreeModel* model) {
  GtkWidget* view;
  GtkTreeSelection* selection;
  GtkCellRenderer* cell;
  GtkTreeViewColumn* column;

  view = gtk_tree_view_new_with_model(model);
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), TRUE);
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
  gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

  cell = gtk_cell_renderer_pixbuf_new();
  column =
      gtk_tree_view_column_new_with_attributes("", cell, "icon-name", 0, NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Source"), cell, "text",
                                                    1, NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Name"), cell, "text", 2,
                                                    NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cell = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Size"), cell, "text", 3,
                                                    NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  g_signal_connect(GTK_WIDGET(view), "button_press_event",
                   G_CALLBACK(RcvTreePopup), this);
  return view;
}

/**
 * 创建已接收文件树底层数据结构.
 * @return FileReceived-model
 */
GtkTreeModel* DialogPeer::CreateFileReceivedModel() {
  GtkListStore* model;

  model = gtk_list_store_new(6, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                             G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);

  return GTK_TREE_MODEL(model);
}
/**
 * 显示信息/文件接收UI(是否显示信息或文件接收).
 *
 */
void DialogPeer::ShowInfoEnclosure(DialogPeer* dlgpr) {
  PalInfo* palinfor;
  GtkTreeModel *mdltorcv, *mdlrcvd, *mdltmp;
  GSList* ecslist;
  GtkWidget *widget, *hpaned, *pbar;
  float progress = 0.0;
  const char* iconname;
  FileInfo* file;
  gchar *filesize, *path;
  char progresstip[MAX_BUFLEN];
  GtkTreeIter iter;
  gint receiving;  // 标记是不是窗口在正传送文件时被关闭，又打开的。

  receiving = 0;

  // 设置界面显示
  palinfor = dlgpr->grpinf->getMembers()[0].get();
  mdltorcv = (GtkTreeModel*)g_datalist_get_data(&(dlgpr->mdlset),
                                                "file-to-receive-model");
  gtk_list_store_clear(GTK_LIST_STORE(mdltorcv));
  mdlrcvd = (GtkTreeModel*)g_datalist_get_data(&(dlgpr->mdlset),
                                               "file-received-model");
  gtk_list_store_clear(GTK_LIST_STORE(mdlrcvd));
  ecslist = dlgpr->app->getCoreThread()->GetPalEnclosure(palinfor);
  if (ecslist) {
    // 只要有该好友的接收文件信息(不分待接收和未接收)，就显示
    hpaned = GTK_WIDGET(g_datalist_get_data(&(dlgpr->widset), "main-paned"));
    widget = GTK_WIDGET(g_datalist_get_data(&(dlgpr->widset), "info-frame"));
    gtk_widget_hide(widget);
    widget = GTK_WIDGET(
        g_datalist_get_data(&(dlgpr->widset), "file-enclosure-frame-widget"));
    gtk_paned_pack2(GTK_PANED(hpaned), widget, FALSE, TRUE);
    widget = GTK_WIDGET(
        g_datalist_get_data(&(dlgpr->widset), "file-receive-paned-widget"));
    gtk_widget_show(widget);
    // 将从中心节点取到的数据向附件接收列表填充
    dlgpr->torcvsize = 0;
    while (ecslist) {
      file = (FileInfo*)ecslist->data;
      filesize = numeric_to_size(file->filesize);
      switch (file->fileattr) {
        case FileAttr::REGULAR:
          iconname = "text-x-generic-symbolic";
          break;
        case FileAttr::DIRECTORY:
          iconname = "folder-symbolic";
          break;
        default:
          iconname = NULL;
          break;
      }
      if (file->finishedsize < file->filesize) {
        file->filepath = ipmsg_get_filename_me(file->filepath, &path);
        if (file->finishedsize > 0)
          receiving += 1;
        mdltmp = mdltorcv;
        dlgpr->torcvsize += file->filesize;
      } else
        mdltmp = mdlrcvd;
      gtk_list_store_append(GTK_LIST_STORE(mdltmp), &iter);
      gtk_list_store_set(GTK_LIST_STORE(mdltmp), &iter, 0, iconname, 1,
                         file->fileown->getName().c_str(), 2, file->filepath, 3,
                         filesize, 5, file, -1);
      g_free(filesize);
      ecslist = g_slist_next(ecslist);
    }
    g_slist_free(ecslist);
    // 设置进度条,如果接收完成重新载入待接收和已接收列表
    if (dlgpr->torcvsize == 0) {
      progress = 0;
      snprintf(progresstip, MAX_BUFLEN, "%s", _("Receiving Progress."));
    } else {
      if (dlgpr->rcvdsize == 0)
        snprintf(progresstip, MAX_BUFLEN, _("%s to Receive."),
                 numeric_to_size(dlgpr->torcvsize));
      else {
        progress = percent(dlgpr->rcvdsize, dlgpr->torcvsize) / 100;
        snprintf(progresstip, MAX_BUFLEN, _("%s Of %s Received."),
                 numeric_to_size(dlgpr->rcvdsize),
                 numeric_to_size(dlgpr->torcvsize));
      }
    }
    if (progress == 1.0) {
      if (dlgpr->timerrcv > 0) {
        g_source_remove(dlgpr->timerrcv);
        dlgpr->timerrcv = 0;
      }
      snprintf(progresstip, MAX_BUFLEN, "%s", _("Mission Completed!"));
    }
    pbar = GTK_WIDGET(g_datalist_get_data(&(dlgpr->widset),
                                          "file-receive-progress-bar-widget"));
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbar), progress);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbar), progresstip);
  } else {
    widget = GTK_WIDGET(
        g_datalist_get_data(&(dlgpr->widset), "file-receive-paned-widget"));
    gtk_widget_hide(widget);
  }

  if (receiving > 0)
    dlgpr->onAcceptButtonClicked(dlgpr);
}
/**
 * 显示窗口事件响应函数.
 *@param dlgpr 对话框类
 *
 */
bool DialogPeer::UpdataEnclosureRcvUI(DialogPeer* dlgpr) {
  GtkTreeModel* model;
  GtkWidget *pbar, *button;
  float progress = 0.0;
  FileInfo* file;
  GtkTreeIter iter;
  char progresstip[MAX_BUFLEN];

  // 处理待接收文件界面显示
  model = (GtkTreeModel*)g_datalist_get_data(&(dlgpr->mdlset),
                                             "file-to-receive-model");
  if (!model) {
    g_source_remove(dlgpr->timerrcv);
    dlgpr->timerrcv = 0;
    return FALSE;
  }
  dlgpr->rcvdsize = 0;
  if (gtk_tree_model_get_iter_first(model, &iter)) {
    do {  // 遍历待接收model
      gtk_tree_model_get(model, &iter, 5, &file, -1);
      if (file->finishedsize == file->filesize) {
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, "tip-finish", -1);
      }
      dlgpr->rcvdsize += file->finishedsize;
    } while (gtk_tree_model_iter_next(model, &iter));
  }
  // 设置进度条,如果接收完成重新载入待接收和已接收列表
  if (dlgpr->torcvsize == 0) {
    progress = 0;
    snprintf(progresstip, MAX_BUFLEN, "%s", _("Receiving Progress."));
  } else {
    if (dlgpr->rcvdsize == 0)
      snprintf(progresstip, MAX_BUFLEN, _("%s to Receive."),
               numeric_to_size(dlgpr->torcvsize));
    else {
      progress = percent(dlgpr->rcvdsize, dlgpr->torcvsize) / 100;
      snprintf(progresstip, MAX_BUFLEN, _("%s Of %s Received."),
               numeric_to_size(dlgpr->rcvdsize),
               numeric_to_size(dlgpr->torcvsize));
    }
  }
  pbar = GTK_WIDGET(g_datalist_get_data(&(dlgpr->widset),
                                        "file-receive-progress-bar-widget"));
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbar), progress);
  gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbar), progresstip);
  if ((progress == 1) || (progress == 0)) {
    if (progress == 1) {
      g_source_remove(dlgpr->timerrcv);
      dlgpr->timerrcv = 0;
      dlgpr->ShowInfoEnclosure(dlgpr);
    }
    // 只要不是在接收过程中，恢复接收和拒收按键
    button = GTK_WIDGET(
        g_datalist_get_data(&(dlgpr->widset), "file-receive-accept-button"));
    gtk_widget_set_sensitive(button, TRUE);
    button = GTK_WIDGET(
        g_datalist_get_data(&(dlgpr->widset), "file-receive-refuse-button"));
    gtk_widget_set_sensitive(button, TRUE);
  } else {
    // 接收过程中，禁止点接收和拒收按键
    button = GTK_WIDGET(
        g_datalist_get_data(&(dlgpr->widset), "file-receive-accept-button"));
    gtk_widget_set_sensitive(button, FALSE);
    button = GTK_WIDGET(
        g_datalist_get_data(&(dlgpr->widset), "file-receive-refuse-button"));
    gtk_widget_set_sensitive(button, FALSE);
  }

  return TRUE;
}
/**
 * 显示窗口事件响应函数.
 *@param dlgpr 对话框类
 *
 */
void DialogPeer::ShowDialogPeer(DialogPeer* dlgpr) {
  // 这个事件有可能需要触发其它功能，暂没有直接用ShowInfoEnclosure来执行
  ShowInfoEnclosure(dlgpr);
}
/**
 * 接收文件函数.
 *@param dlgpr 对话框类
 *
 */
void DialogPeer::onAcceptButtonClicked(DialogPeer* self) {
  GtkWidget* widget;
  GtkTreeModel* model;
  GtkTreeIter iter;
  gchar* filename;
  FileInfo* file;

  auto g_progdt = self->app->getCoreThread()->getProgramData();

  const gchar* filepath = pop_save_path(GTK_WIDGET(self->grpinf->getDialog()),
                                        g_progdt->path.c_str());
  if (filepath == nullptr) {
    return;
  }
  self->progdt->path = filepath;
  /* 考察数据集中是否存在项 */
  widget = self->fileToReceiveTreeviewWidget;
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  if (!model)
    return;
  if (!gtk_tree_model_get_iter_first(model, &iter))
    return;
  self->torcvsize = 0;
  /* 将选中的项投入文件数据接收类 */
  do {
    gtk_tree_model_get(model, &iter, 2, &filename, 5, &file, -1);
    g_free(file->filepath);
    file->filepath = g_strdup_printf(
        "%s%s%s", filepath, *(filepath + 1) != '\0' ? "/" : "", filename);
    self->app->getCoreThread()->RecvFileAsync(file);
    g_free(filename);
    self->torcvsize += file->filesize;
  } while (gtk_tree_model_iter_next(model, &iter));
  self->rcvdsize = 0;
  if (!self->timerrcv) {
    self->timerrcv =
        g_timeout_add(300, GSourceFunc(UpdataEnclosureRcvUI), self);
  }
}

/**
 * 获取待发送成员列表.
 * @return plist 获取待发送成员列表
 * 调用该函数后须free plist
 */
GSList* DialogPeer::GetSelPal() {
  PalInfo* pal;
  GSList* plist;
  pal = grpinf->getMembers()[0].get();
  plist = NULL;
  plist = g_slist_append(plist, pal);
  return plist;
}

/**
 *从接收文件的TreeView删除选定行（待接收和已接收都用此函数）.
 * @param widget TreeView
 */
void DialogPeer::onRefuse(void*, void*, DialogPeer& self) {
  GtkTreeModel* model;
  GtkTreeSelection* TreeSel;
  GtkTreeIter iter;
  FileInfo* file;
  GList* list;

  GtkWidget* widget = self.fileToReceiveTreeviewWidget;
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
  // 从中心结点删除
  TreeSel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
  list = gtk_tree_selection_get_selected_rows(TreeSel, NULL);
  if (!list)
    return;
  while (list) {
    gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &iter,
                            (GtkTreePath*)g_list_nth(list, 0)->data);
    gtk_tree_model_get(model, &iter, 5, &file, -1);
    self.app->getCoreThread()->PopItemFromEnclosureList(file);
    list = g_list_next(list);
  }
  g_list_free(list);
  // 从列表中删除
  RemoveSelectedFromTree(widget);
  // 重新刷新窗口显示
  self.ShowInfoEnclosure(&self);
}

void DialogPeer::onRefuseAll(void*, void*, DialogPeer& self) {
  GtkTreeModel* model;
  GtkTreeIter iter;
  FileInfo* file;

  GtkWidget* widget = self.fileToReceiveTreeviewWidget;
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));

  if (gtk_tree_model_get_iter_first(model, &iter)) {
    do {
      gtk_tree_model_get(model, &iter, 5, &file, -1);
      self.app->getCoreThread()->PopItemFromEnclosureList(file);
    } while (gtk_tree_model_iter_next(model, &iter));
  }
  gtk_list_store_clear(GTK_LIST_STORE(model));
}

/**
 *显示接收附件的TreeView的弹出菜单回调函数.(待接收和已接收都用此函数)
 * @param widget TreeView
 * @param event 事件
 */
gint DialogPeer::RcvTreePopup(GtkWidget* widget,
                              GdkEvent* event,
                              DialogPeer* self) {
  GtkWidget* menu;

  menu = gtk_menu_new_from_model(G_MENU_MODEL(
      gtk_builder_get_object(self->app->getMenuBuilder(), "peer-recv-popup")));
  gtk_menu_attach_to_widget(GTK_MENU(menu), widget, nullptr);

  if (event->type == GDK_BUTTON_PRESS) {
    if (gdk_event_triggers_context_menu(event)) {
      gtk_menu_popup_at_pointer(GTK_MENU(menu), NULL);
      return TRUE;
    }
  }
  return FALSE;
}

void DialogPeer::onNewFileReceived(GroupInfo*) {
  this->ShowInfoEnclosure(this);
}

void DialogPeer::onGroupInfoUpdated(GroupInfo* groupInfo) {
  if (groupInfo != this->grpinf)
    return;
  if (gtk_window_is_active(GTK_WINDOW(this->window))) {
    ClearNotify(GTK_WIDGET(this->window), nullptr);
  }
}

void DialogPeer::refreshSendAction() {
  bool can_send = false;
  if (gtk_text_buffer_get_char_count(getInputBuffer()) > 0) {
    can_send = true;
  } else {
    GtkTreeIter iter;
    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(fileSendModel), &iter)) {
      can_send = true;
    }
  }

  if (can_send) {
    g_action_map_enable_actions(G_ACTION_MAP(window), "send_message", nullptr);
  } else {
    g_action_map_disable_actions(G_ACTION_MAP(window), "send_message", nullptr);
  }
}

}  // namespace iptux
