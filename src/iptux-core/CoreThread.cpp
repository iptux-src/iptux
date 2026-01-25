#include "config.h"
#include "iptux-core/CoreThread.h"

#include "Const.h"
#include "gio/gio.h"
#include <cassert>
#include <cstdio>
#include <deque>
#include <fstream>
#include <functional>
#include <future>
#include <list>
#include <memory>
#include <mutex>
#include <thread>

#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <sys/socket.h>

#include "iptux-core/internal/Command.h"
#include "iptux-core/internal/RecvFileData.h"
#include "iptux-core/internal/SendFile.h"
#include "iptux-core/internal/TcpData.h"
#include "iptux-core/internal/UdpDataService.h"
#include "iptux-core/internal/ipmsg.h"
#include "iptux-core/internal/support.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"
#include <unistd.h>

using namespace std;
using namespace std::placeholders;

namespace iptux {

// MARK: enum CoreThreadErr

const char* coreThreadErrToStr(enum CoreThreadErr err) {
  switch (err) {
    case CORE_THREAD_ERR_NONE:
      return "No error";
    case CORE_THREAD_ERR_STARTED_TWICE:
      return "Core thread started twice";
    case CORE_THREAD_ERR_SOCKET_CREATE_FAILED:
      return _("Socket create failed");
    case CORE_THREAD_ERR_UDP_BIND_FAILED:
      return _("UDP bind failed");
    case CORE_THREAD_ERR_UDP_THREAD_START_FAILED:
      return _("UDP thread start failed");
    case CORE_THREAD_ERR_TCP_BIND_FAILED:
      return _("TCP bind failed");
    case CORE_THREAD_ERR_TCP_THREAD_START_FAILED:
      return _("TCP thread start failed");
    default:
      return _("Unknown error");
  }
}

// MARK: UDP Thread

struct UdpThread;
struct UdpThreadOps {
  bool (*on_new_msg)(UdpThread* udpThread,
                     GSocketAddress* peer,
                     const char* msg,
                     size_t size);
  void (*on_init_failed)(UdpThread* udpThread);
};

enum UdpThreadState {
  UDP_THREAD_STATE_INIT = 0,
  UDP_THREAD_STATE_RUNNING,
  UDP_THREAD_STATE_CLOSING,
  UDP_THREAD_STATE_CLOSED
};

struct UdpThread {
  // config
  GSocket* socket = nullptr;
  void* data = nullptr;
  const UdpThreadOps* ops = nullptr;
  // runtime
  atomic<enum UdpThreadState> state = UDP_THREAD_STATE_INIT;
  GMainLoop* loop = nullptr;
  GThread* thread = nullptr;
  GMainContext* ctx = nullptr;
  guint id = 0;

  UdpThread() = default;
  ~UdpThread();
};

void udpThreadStop(UdpThread* udpThread);
UdpThread::~UdpThread() {
  if (state != UDP_THREAD_STATE_CLOSED && state != UDP_THREAD_STATE_INIT) {
    udpThreadStop(this);
  }
  assert(state == UDP_THREAD_STATE_CLOSED);
  assert(thread == nullptr);
  if (loop) {
    g_main_loop_unref(loop);
    loop = nullptr;
  }
  if (ctx) {
    g_main_context_unref(ctx);
    ctx = nullptr;
  }
}

gboolean udpThreadCb(GIOChannel*, GIOCondition condition, gpointer data) {
  UdpThread* udpThread = static_cast<UdpThread*>(data);

  if (condition & (G_IO_HUP | G_IO_ERR)) {
    LOG_ERROR("UDP socket closed or error in udpThreadCb (condition=0x%x)", condition);
    return FALSE;  // remove source
  }

  if (condition & G_IO_IN) {
    char buf[MAX_UDPLEN];
    GSocketAddress* peer;
    GError* error = nullptr;

    gssize n = g_socket_receive_from(udpThread->socket, &peer, buf,
                                     sizeof(buf) - 1, NULL, /* GCancellable */
                                     &error);
    if (n < 0) {
      LOG_ERROR("g_socket_receive_from failed: %s", error->message);
      g_error_free(error);
      return TRUE;  // keep watching
    }

    if (n == 0) {
      g_object_unref(peer);
      return TRUE;  // keep watching
    }

    buf[n] = '\0';
    LOG_INFO("Received %zd bytes: %s\n", n, buf);
    if (!udpThread->ops->on_new_msg(udpThread, peer, buf, n)) {
      LOG_WARN("udpThreadCb on_new_msg failed");
    }
    g_object_unref(peer);
  }

  return TRUE;  // keep watching
}

gboolean udpThreadAttachUdp(UdpThread* udpThread) {
  GSource* src = g_socket_create_source(
      udpThread->socket, (GIOCondition)(G_IO_IN | G_IO_ERR | G_IO_HUP), NULL);
  g_source_set_callback(src, (GSourceFunc)udpThreadCb, udpThread, NULL);
  udpThread->id = g_source_attach(src, udpThread->ctx);
  g_source_unref(src);

  if (udpThread->id == 0) {
    LOG_ERROR(
        "g_source_attach failed, unable to attach UDP socket to main loop");
    return FALSE;
  }
  return TRUE;
}

gboolean udpThreadStop0(gpointer data) {
  UdpThread* udpThread = static_cast<UdpThread*>(data);

  LOG_DEBUG("Quitting UDP thread loop");
  g_main_loop_quit(udpThread->loop);
  return FALSE;
}

void udpThreadStop(UdpThread* udpThread) {
  if (udpThread->state == UDP_THREAD_STATE_CLOSED) {
    LOG_WARN("UDP thread already closed");
    return;
  }
  if (udpThread->state == UDP_THREAD_STATE_INIT) {
    LOG_WARN("UDP thread not started");
    return;
  }
  if (udpThread->state == UDP_THREAD_STATE_RUNNING) {
    g_main_context_invoke(udpThread->ctx, udpThreadStop0, udpThread);
  }
  (void)g_thread_join(udpThread->thread);
  udpThread->thread = nullptr;
  udpThread->state = UDP_THREAD_STATE_CLOSED;
  LOG_INFO("UDP thread stopped");
}

static bool udpThreadRun0(UdpThread* udpThread) {
  if (!g_main_context_acquire(udpThread->ctx)) {
    LOG_ERROR("g_main_context_acquire failed");
    return false;
  }
  g_main_context_push_thread_default(udpThread->ctx);

  udpThread->loop = g_main_loop_new(udpThread->ctx, FALSE);
  if (!udpThreadAttachUdp(udpThread)) {
    LOG_ERROR("udpThreadAttachUdp failed");
    return false;
  }
  return true;
}

static gpointer udpThreadRun(gpointer data) {
  UdpThread* udpThread = static_cast<UdpThread*>(data);

  if (!udpThreadRun0(udpThread)) {
    LOG_ERROR("Failed to init UDP thread");
    udpThread->state = UDP_THREAD_STATE_CLOSING;
    if (udpThread->ops->on_init_failed) {
      udpThread->ops->on_init_failed(udpThread);
    }
    return NULL;
  }

  LOG_INFO("UDP thread start");
  g_main_loop_run(udpThread->loop);
  g_main_context_pop_thread_default(udpThread->ctx);
  g_main_loop_unref(udpThread->loop);
  g_main_context_unref(udpThread->ctx);
  udpThread->loop = nullptr;
  udpThread->ctx = nullptr;
  udpThread->state = UDP_THREAD_STATE_CLOSING;
  return NULL;
}

gboolean udpThreadStart(UdpThread* udpThread) {
  if (udpThread->state != UDP_THREAD_STATE_INIT) {
    LOG_WARN("UDP thread already started");
    return FALSE;
  }

  udpThread->ctx = g_main_context_new();
  udpThread->thread = g_thread_new("udpThread", udpThreadRun, udpThread);
  udpThread->state = UDP_THREAD_STATE_RUNNING;
  if (!udpThread->thread) {
    LOG_ERROR("Failed to create UDP thread");
    udpThread->state = UDP_THREAD_STATE_CLOSED;
    return FALSE;
  }
  return TRUE;
}

// ============================================================================
// TCP Thread Implementation
// ============================================================================

struct TcpThread;
struct TcpThreadOps {
  void (*on_new_connection)(TcpThread* tcpThread, GSocket* clientSocket);
};

enum TcpThreadState {
  TCP_THREAD_STATE_INIT = 0,
  TCP_THREAD_STATE_RUNNING,
  TCP_THREAD_STATE_CLOSING,
  TCP_THREAD_STATE_CLOSED
};

struct TcpThread {
  // config
  GSocket* socket = nullptr;
  void* data = nullptr;
  const TcpThreadOps* ops = nullptr;
  // runtime
  atomic<enum TcpThreadState> state = TCP_THREAD_STATE_INIT;
  GMainLoop* loop = nullptr;
  GThread* thread = nullptr;
  GMainContext* ctx = nullptr;
  guint id = 0;

  TcpThread() = default;
  ~TcpThread();
};

void tcpThreadStop(TcpThread* tcpThread);

TcpThread::~TcpThread() {
  if (state != TCP_THREAD_STATE_CLOSED && state != TCP_THREAD_STATE_INIT) {
    tcpThreadStop(this);
  }
  assert(state == TCP_THREAD_STATE_CLOSED || state == TCP_THREAD_STATE_INIT);
  assert(thread == nullptr);
  if (loop) {
    g_main_loop_unref(loop);
    loop = nullptr;
  }
  if (ctx) {
    g_main_context_unref(ctx);
    ctx = nullptr;
  }
}

gboolean tcpThreadCb(GSocket* socket, GIOCondition condition, gpointer data) {
  TcpThread* tcpThread = static_cast<TcpThread*>(data);

  if (condition & (G_IO_HUP | G_IO_ERR)) {
    LOG_ERROR("TCP socket closed or error in tcpThreadCb (condition=0x%x)",
              condition);
    return FALSE;  // remove source
  }

  if (condition & G_IO_IN) {
    GError* error = nullptr;
    GSocket* clientSocket = g_socket_accept(socket, nullptr, &error);
    if (!clientSocket) {
      if (error) {
        LOG_WARN("g_socket_accept failed: %s", error->message);
        g_error_free(error);
      }
      return TRUE;  // keep watching
    }

    tcpThread->ops->on_new_connection(tcpThread, clientSocket);
  }

  return TRUE;  // keep watching
}

gboolean tcpThreadAttachTcp(TcpThread* tcpThread) {
  GError* error = nullptr;
  if (!g_socket_listen(tcpThread->socket, &error)) {
    LOG_ERROR("g_socket_listen failed: %s", error->message);
    g_error_free(error);
    return FALSE;
  }

  GSource* src = g_socket_create_source(
      tcpThread->socket, (GIOCondition)(G_IO_IN | G_IO_ERR | G_IO_HUP), NULL);
  g_source_set_callback(src, (GSourceFunc)tcpThreadCb, tcpThread, NULL);
  tcpThread->id = g_source_attach(src, tcpThread->ctx);
  g_source_unref(src);

  if (tcpThread->id == 0) {
    LOG_ERROR(
        "g_source_attach failed, unable to attach TCP socket to main loop");
    return FALSE;
  }
  return TRUE;
}

gboolean tcpThreadStop0(gpointer data) {
  TcpThread* tcpThread = static_cast<TcpThread*>(data);

  LOG_DEBUG("Quitting TCP thread loop");
  g_main_loop_quit(tcpThread->loop);
  return FALSE;
}

void tcpThreadStop(TcpThread* tcpThread) {
  if (tcpThread->state == TCP_THREAD_STATE_CLOSED) {
    LOG_WARN("TCP thread already closed");
    return;
  }
  if (tcpThread->state == TCP_THREAD_STATE_INIT) {
    LOG_WARN("TCP thread not started");
    tcpThread->state = TCP_THREAD_STATE_CLOSED;
    return;
  }
  if (tcpThread->state == TCP_THREAD_STATE_RUNNING) {
    g_main_context_invoke(tcpThread->ctx, tcpThreadStop0, tcpThread);
  }
  (void)g_thread_join(tcpThread->thread);
  tcpThread->thread = nullptr;
  tcpThread->state = TCP_THREAD_STATE_CLOSED;
  LOG_INFO("TCP thread stopped");
}

static bool tcpThreadRun0(TcpThread* tcpThread) {
  if (!g_main_context_acquire(tcpThread->ctx)) {
    LOG_ERROR("g_main_context_acquire failed");
    return false;
  }
  g_main_context_push_thread_default(tcpThread->ctx);

  tcpThread->loop = g_main_loop_new(tcpThread->ctx, FALSE);
  if (!tcpThreadAttachTcp(tcpThread)) {
    LOG_ERROR("tcpThreadAttachTcp failed");
    return false;
  }
  return true;
}

static gpointer tcpThreadRun(gpointer data) {
  TcpThread* tcpThread = static_cast<TcpThread*>(data);

  if (!tcpThreadRun0(tcpThread)) {
    LOG_ERROR("Failed to init TCP thread");
    tcpThread->state = TCP_THREAD_STATE_CLOSING;
    return NULL;
  }

  LOG_INFO("TCP thread start");
  g_main_loop_run(tcpThread->loop);
  g_main_context_pop_thread_default(tcpThread->ctx);
  g_main_loop_unref(tcpThread->loop);
  g_main_context_unref(tcpThread->ctx);
  tcpThread->loop = nullptr;
  tcpThread->ctx = nullptr;
  tcpThread->state = TCP_THREAD_STATE_CLOSING;
  return NULL;
}

gboolean tcpThreadStart(TcpThread* tcpThread) {
  if (tcpThread->state != TCP_THREAD_STATE_INIT) {
    LOG_WARN("TCP thread already started");
    return FALSE;
  }

  tcpThread->ctx = g_main_context_new();
  tcpThread->thread = g_thread_new("tcpThread", tcpThreadRun, tcpThread);
  tcpThread->state = TCP_THREAD_STATE_RUNNING;
  if (!tcpThread->thread) {
    LOG_ERROR("Failed to create TCP thread");
    tcpThread->state = TCP_THREAD_STATE_CLOSED;
    return FALSE;
  }
  return TRUE;
}

namespace {
/**
 * 初始化程序iptux的运行环境.
 * cache iptux {pic, photo, icon} \n
 * config iptux {log, photo, icon} \n
 */
void init_iptux_environment() {
  const char* env;
  char path[MAX_PATHLEN];

  env = g_get_user_cache_dir();
  if (access(env, F_OK) != 0)
    g_mkdir(env, 0755);
  snprintf(path, MAX_PATHLEN, "%s" IPTUX_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0755);
  snprintf(path, MAX_PATHLEN, "%s" PIC_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0755);
  snprintf(path, MAX_PATHLEN, "%s" PHOTO_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0755);
  snprintf(path, MAX_PATHLEN, "%s" ICON_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0755);
  snprintf(path, MAX_PATHLEN, "%s" LOG_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0755);
  snprintf(path, MAX_PATHLEN, "%s" SENT_IMAGE_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0755);

  env = g_get_user_config_dir();
  if (access(env, F_OK) != 0)
    g_mkdir(env, 0777);
  snprintf(path, MAX_PATHLEN, "%s" IPTUX_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0777);
  snprintf(path, MAX_PATHLEN, "%s" LOG_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0777);
  snprintf(path, MAX_PATHLEN, "%s" PHOTO_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0777);
  snprintf(path, MAX_PATHLEN, "%s" ICON_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0777);
  snprintf(path, MAX_PATHLEN, "%s" LOG_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0777);
}

}  // namespace

// MARK: CoreThread
struct CoreThread::Impl {
  CoreThread* owner{nullptr};  // Back-pointer to owner
  uint16_t port;

  PPalInfo me;

  UdpDataService_U udp_data_service;

  GSList* blacklist{nullptr};  // 黑名单链表
  bool debugDontBroadcast{false};
  vector<shared_ptr<PalInfo>> pallist;  // 好友链表(成员不能被删除)

  map<uint32_t, shared_ptr<FileInfo>> privateFiles;
  int lastTransTaskId{0};
  int eventCount{0};
  shared_ptr<const Event> lastEvent{nullptr};
  map<int, shared_ptr<TransAbstract>> transTasks;
  deque<shared_ptr<const Event>> waitingEvents;
  std::mutex waitingEventsMutex;

  future<void> notifyToAllFuture;

  UdpThread* udpThread{nullptr};
  TcpThread* tcpThread{nullptr};
  enum CoreThreadErr lastErr;
  GSocket* udpSocket{nullptr};
  GSocket* tcpSocket{nullptr};
  bool ignoreTcpBindFailed{false};

  // TCP handler threads for incoming connections
  std::list<GThread*> tcpHandlerThreads;
  std::mutex tcpHandlerThreadsMutex;

  Impl() = default;
  ~Impl();

  std::list<GThread*>::iterator addTcpHandlerThread(GThread* thread);
  void removeTcpHandlerThread(std::list<GThread*>::iterator it);
  void joinAllTcpHandlerThreads();
};

CoreThread::Impl::~Impl() {
  // Join any remaining TCP handler threads
  joinAllTcpHandlerThreads();

  if (udpThread) {
    delete udpThread;
    udpThread = nullptr;
  }
  if (tcpThread) {
    delete tcpThread;
    tcpThread = nullptr;
  }
  if (udpSocket) {
    g_object_unref(udpSocket);
    udpSocket = nullptr;
  }
  if (tcpSocket) {
    g_object_unref(tcpSocket);
    tcpSocket = nullptr;
  }
}

std::list<GThread*>::iterator CoreThread::Impl::addTcpHandlerThread(GThread* thread) {
  std::lock_guard<std::mutex> lock(tcpHandlerThreadsMutex);
  tcpHandlerThreads.push_back(thread);
  return std::prev(tcpHandlerThreads.end());
}

void CoreThread::Impl::removeTcpHandlerThread(std::list<GThread*>::iterator it) {
  std::lock_guard<std::mutex> lock(tcpHandlerThreadsMutex);
  GThread* thread = *it;
  tcpHandlerThreads.erase(it);
  g_thread_join(thread);
}

void CoreThread::Impl::joinAllTcpHandlerThreads() {
  std::lock_guard<std::mutex> lock(tcpHandlerThreadsMutex);
  for (GThread* thread : tcpHandlerThreads) {
    if (thread) {
      g_thread_join(thread);
    }
  }
  tcpHandlerThreads.clear();
}

CoreThread::CoreThread(shared_ptr<ProgramData> data)
    : programData(data),
      config(data->getConfig()),
      started(false),
      pImpl(std::make_unique<Impl>()) {
  pImpl->owner = this;
  if (config->GetBool("debug_dont_broadcast")) {
    pImpl->debugDontBroadcast = true;
  }
  pImpl->port = programData->port();
  pImpl->udp_data_service = make_unique<UdpDataService>(*this);
  pImpl->me = make_shared<PalInfo>("127.0.0.1", port());
  (*pImpl->me)
      .setUser(g_get_user_name())
      .setHost(g_get_host_name())
      .setName(programData->nickname)
      .setGroup(programData->mygroup)
      .setEncode("utf-8")
      .setCompatible(true);
}

CoreThread::~CoreThread() {
  if (started) {
    stop();
  }
  g_slist_free(pImpl->blacklist);
}

bool udpThreadOpsOnNewMsg(UdpThread* udpThread,
                          GSocketAddress* peer,
                          const char* msg,
                          size_t size) {
  CoreThread::Impl* self = static_cast<CoreThread::Impl*>(udpThread->data);

  try {
    self->udp_data_service->process(peer, msg, size);
  } catch (const std::exception& e) {
    LOG_ERROR("Exception in UDP message processing: %s", e.what());
    return false;
  } catch (...) {
    LOG_ERROR("Unknown exception in UDP message processing");
    return false;
  }
  return true;
}

void udpThreadOpsOnInitFailed(UdpThread* udpThread) {
  CoreThread::Impl* self = static_cast<CoreThread::Impl*>(udpThread->data);
  self->lastErr = CORE_THREAD_ERR_UDP_THREAD_START_FAILED;
  // TODO: notify core thread
}

static const UdpThreadOps udpThreadOps = {
    .on_new_msg = udpThreadOpsOnNewMsg,
    .on_init_failed = udpThreadOpsOnInitFailed,
};

// Data passed to TCP handler thread
struct TcpHandlerData {
  CoreThread* coreThread;
  CoreThread::Impl* impl;
  GSocket* clientSocket;
  std::list<GThread*>::iterator threadIt;  // Set after thread creation
  bool threadItValid{false};
};

// Cleanup callback invoked on TCP thread's main context
static gboolean tcpHandlerCleanupCb(gpointer data) {
  TcpHandlerData* handlerData = static_cast<TcpHandlerData*>(data);
  if (handlerData->threadItValid) {
    handlerData->impl->removeTcpHandlerThread(handlerData->threadIt);
  }
  delete handlerData;
  return FALSE;
}

static gpointer tcpHandlerThreadFunc(gpointer data) {
  TcpHandlerData* handlerData = static_cast<TcpHandlerData*>(data);
  try {
    TcpData::TcpDataEntry(handlerData->coreThread, handlerData->clientSocket);
  } catch (const std::exception& e) {
    LOG_ERROR("Exception in TCP handler: %s", e.what());
  } catch (...) {
    LOG_ERROR("Unknown exception in TCP handler");
  }

  // Schedule cleanup on TCP thread's main context
  if (handlerData->impl->tcpThread &&
      handlerData->impl->tcpThread->ctx) {
    g_main_context_invoke(handlerData->impl->tcpThread->ctx,
                          tcpHandlerCleanupCb, handlerData);
  } else {
    // TCP thread already stopped, just delete
    delete handlerData;
  }
  return nullptr;
}

void tcpThreadOpsOnNewConnection(TcpThread* tcpThread, GSocket* clientSocket) {
  CoreThread::Impl* impl = static_cast<CoreThread::Impl*>(tcpThread->data);

  // Create handler data (will be freed by cleanup callback)
  TcpHandlerData* handlerData = new TcpHandlerData();
  handlerData->coreThread = impl->owner;
  handlerData->impl = impl;
  handlerData->clientSocket = clientSocket;

  // Create a GThread to handle the connection
  GThread* thread =
      g_thread_new("tcp-handler", tcpHandlerThreadFunc, handlerData);
  if (thread) {
    handlerData->threadIt = impl->addTcpHandlerThread(thread);
    handlerData->threadItValid = true;
  } else {
    LOG_ERROR("Failed to create TCP handler thread");
    delete handlerData;
    g_object_unref(clientSocket);
  }
}

static const TcpThreadOps tcpThreadOps = {
    .on_new_connection = tcpThreadOpsOnNewConnection,
};

/**
 * 程序核心入口，主要任务服务将在此开启.
 */
bool CoreThread::start() noexcept {
  if (started) {
    pImpl->lastErr = CORE_THREAD_ERR_STARTED_TWICE;
    return false;
  }
  started = true;
  init_iptux_environment();
  if (!bind_iptux_port())
    return false;

  pImpl->udpThread = new UdpThread();
  pImpl->udpThread->socket = pImpl->udpSocket;
  pImpl->udpThread->ops = &udpThreadOps;
  pImpl->udpThread->data = pImpl.get();
  if (!udpThreadStart(pImpl->udpThread)) {
    pImpl->lastErr = CORE_THREAD_ERR_UDP_THREAD_START_FAILED;
    LOG_ERROR("Failed to start UDP thread");
    return false;
  }

  if (pImpl->tcpSocket) {
    pImpl->tcpThread = new TcpThread();
    pImpl->tcpThread->socket = pImpl->tcpSocket;
    pImpl->tcpThread->ops = &tcpThreadOps;
    pImpl->tcpThread->data = pImpl.get();
    if (!tcpThreadStart(pImpl->tcpThread)) {
      pImpl->lastErr = CORE_THREAD_ERR_TCP_THREAD_START_FAILED;
      LOG_ERROR("Failed to start TCP thread");
      return false;
    }
  }

  pImpl->notifyToAllFuture =
      async([](CoreThread* ct) { SendNotifyToAll(ct); }, this);
  return true;
}

static GSocket* bind_udp_port(const char* ip, uint16_t port) {
  GError* error = nullptr;
  GSocket* udpSock = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_DATAGRAM,
                                  G_SOCKET_PROTOCOL_UDP, &error);
  if (error != nullptr) {
    LOG_ERROR("g_socket_new failed: %s", error->message);
    g_error_free(error);
    return nullptr;
  }
  g_socket_set_broadcast(udpSock, TRUE);

  GSocketAddress* bind_addr = g_inet_socket_address_new_from_string(ip, port);
  if (!bind_addr) {
    g_object_unref(udpSock);
    LOG_ERROR("create bind address failed: %s:%d", ip, port);
    return nullptr;
  }

  if (!g_socket_bind(udpSock, bind_addr, TRUE, &error)) {
    auto errmsg =
        stringFormat(_("Fatal Error!! Failed to bind the UDP port(%s:%d)!\n%s"),
                     ip, port, error->message);
    g_error_free(error);
    LOG_ERROR("%s", errmsg.c_str());
    g_object_unref(bind_addr);
    g_object_unref(udpSock);
    return nullptr;
  }
  g_object_unref(bind_addr);
  LOG_INFO("bind UDP port(%s:%d) success.", ip, port);
  return udpSock;
}

static GSocket* bind_tcp_port(const char* ip, uint16_t port) {
  GError* error = nullptr;
  GSocket* tcpSock = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_STREAM,
                                  G_SOCKET_PROTOCOL_TCP, &error);
  if (error != nullptr) {
    LOG_ERROR("g_socket_new failed: %s", error->message);
    g_error_free(error);
    return nullptr;
  }

  // Enable SO_REUSEADDR
  if (!g_socket_set_option(tcpSock, SOL_SOCKET, SO_REUSEADDR, 1, &error)) {
    LOG_WARN("g_socket_set_option for SO_REUSEADDR failed: %s", error->message);
    g_error_free(error);
    error = nullptr;
  }

  GSocketAddress* bind_addr = g_inet_socket_address_new_from_string(ip, port);
  if (!bind_addr) {
    g_object_unref(tcpSock);
    LOG_ERROR("create bind address failed: %s:%d", ip, port);
    return nullptr;
  }

  if (!g_socket_bind(tcpSock, bind_addr, TRUE, &error)) {
    auto errmsg =
        stringFormat(_("Fatal Error!! Failed to bind the TCP port(%s:%d)!\n%s"),
                     ip, port, error->message);
    g_error_free(error);
    LOG_ERROR("%s", errmsg.c_str());
    g_object_unref(bind_addr);
    g_object_unref(tcpSock);
    return nullptr;
  }
  g_object_unref(bind_addr);
  LOG_INFO("bind TCP port(%s:%d) success.", ip, port);
  return tcpSock;
}

void CoreThread::setIgnoreTcpBindFailed(bool ignore) {
  pImpl->ignoreTcpBindFailed = ignore;
}

size_t CoreThread::getTcpHandlerThreadCount() const {
  std::lock_guard<std::mutex> lock(pImpl->tcpHandlerThreadsMutex);
  return pImpl->tcpHandlerThreads.size();
}

bool CoreThread::bind_iptux_port() noexcept {
  auto bind_ip = config->GetString("bind_ip", "0.0.0.0");
  uint16_t port = programData->port();

  pImpl->udpSocket = bind_udp_port(bind_ip.c_str(), port);
  if (!pImpl->udpSocket) {
    pImpl->lastErr = CORE_THREAD_ERR_UDP_BIND_FAILED;
    return false;
  }

  pImpl->tcpSocket = bind_tcp_port(bind_ip.c_str(), port);
  if (!pImpl->tcpSocket) {
    if (pImpl->ignoreTcpBindFailed) {
      LOG_WARN("TCP bind failed but ignored due to ignoreTcpBindFailed flag");
    } else {
      pImpl->lastErr = CORE_THREAD_ERR_TCP_BIND_FAILED;
      g_object_unref(pImpl->udpSocket);
      pImpl->udpSocket = nullptr;
      return false;
    }
  }

  return true;
}

void CoreThread::stop() {
  if (!started) {
    throw "CoreThread not started, or already stopped";
  }
  started = false;
  ClearSublayer();
  if (pImpl->tcpThread) {
    tcpThreadStop(pImpl->tcpThread);
  }
  if (pImpl->udpThread) {
    udpThreadStop(pImpl->udpThread);
  }
  // Join all TCP handler threads after stopping the accept thread
  pImpl->joinAllTcpHandlerThreads();
  pImpl->notifyToAllFuture.wait();
}

uint16_t CoreThread::port() const {
  return pImpl->port;
}

void CoreThread::ClearSublayer() {
  /**
   * @note 必须在发送下线信息之后才能关闭套接口.
   * Socket closing is handled by the thread destructors and Impl destructor.
   */
  for (auto palInfo : pImpl->pallist) {
    SendBroadcastExit(palInfo);
  }
}

int CoreThread::getUdpSock() const {
  if (!pImpl->udpSocket) {
    return -1;
  }
  // TODO: should no longer use it
  return g_socket_get_fd(pImpl->udpSocket);
}

GSocket* CoreThread::getUdpSocket() const {
  return pImpl->udpSocket;
}

int CoreThread::getTcpSock() const {
  if (!pImpl->tcpSocket) {
    return -1;
  }
  // TODO: should no longer use it
  return g_socket_get_fd(pImpl->tcpSocket);
}

shared_ptr<ProgramData> CoreThread::getProgramData() {
  return programData;
}

/**
 * 向局域网内所有计算机发送上线信息.
 * @param pcthrd 核心类
 */
void CoreThread::SendNotifyToAll(CoreThread* pcthrd) {
  Command cmd(*pcthrd);
  if (!pcthrd->pImpl->debugDontBroadcast) {
    cmd.BroadCast(pcthrd->getUdpSocket(), pcthrd->port());
  }
  cmd.DialUp(pcthrd->getUdpSock(), pcthrd->port());
}

/**
 * 黑名单链表中是否包含此项.
 * @param ipv4 ipv4
 * @return 是否包含
 */
bool CoreThread::BlacklistContainItem(in_addr ipv4) const {
  return g_slist_find(pImpl->blacklist, GUINT_TO_POINTER(ipv4.s_addr));
}

bool CoreThread::IsBlocked(in_addr ipv4) const {
  return programData->IsUsingBlacklist() and BlacklistContainItem(ipv4);
}

void CoreThread::Lock() const {
  mutex.lock();
}

void CoreThread::Unlock() const {
  mutex.unlock();
}

/**
 * 获取好友链表.
 * @return 好友链表
 */
const vector<shared_ptr<PalInfo>>& CoreThread::GetPalList() {
  return pImpl->pallist;
}

/**
 * 从好友链表中移除所有好友数据(非UI线程安全).
 * @note 鉴于好友链表成员不能被删除，所以将成员改为下线标记即可
 */
void CoreThread::ClearAllPalFromList() {
  /* 清除所有好友的在线标志 */
  for (auto palInfo : pImpl->pallist) {
    palInfo->setOnline(false);
  }
}

shared_ptr<PalInfo> CoreThread::GetPal(PalKey palKey) {
  for (auto palInfo : pImpl->pallist) {
    if (ipv4Equal(palInfo->ipv4(), palKey.GetIpv4())) {
      return palInfo;
    }
  }
  return {};
}

shared_ptr<PalInfo> CoreThread::GetPal(const string& ipv4) {
  return GetPal(PalKey(inAddrFromString(ipv4), port()));
}

/**
 * 从好友链表中删除指定的好友信息数据(非UI线程安全).
 * @param ipv4 ipv4
 * @note 鉴于好友链表成员不能被删除，所以将成员改为下线标记即可；
 * 鉴于群组中只能包含在线的好友，所以若某群组中包含了此好友，则必须从此群组中删除此好友
 */
void CoreThread::DelPalFromList(PalKey palKey) {
  PPalInfo pal;

  /* 获取好友信息数据，并将其置为下线状态 */
  if (!(pal = GetPal(palKey)))
    return;
  pal->setOnline(false);
  emitEvent(make_shared<const PalOfflineEvent>(palKey));
}

/**
 * 通告指定的好友信息数据已经被更新(非UI线程安全).
 * @param ipv4 ipv4
 * @note 什么时候会用到？1、好友更新个人资料；2、好友下线后又上线了
 * @note 鉴于群组中必须包含所有属于自己的成员，移除不属于自己的成员，
 * 所以好友信息更新后应该重新调整群组成员；
 * @note 群组中被更新的成员信息也应该在界面上做出相应更新
 */
void CoreThread::UpdatePalToList(PalKey palKey) {
  PPalInfo pal;
  /* 如果好友链表中不存在此好友，则视为程序设计出错 */
  if (!(pal = GetPal(palKey))) {
    return;
  }
  pal->setOnline(true);
  emitEvent(make_shared<const PalUpdateEvent>(pal));
}

/**
 * 将好友信息数据加入到好友链表(非UI线程安全).
 * @param pal class PalInfo
 * @note 鉴于在线的好友必须被分配到它所属的群组，所以加入好友到好友链表的同时
 * 也应该分配好友到相应的群组
 */
void CoreThread::AttachPalToList(shared_ptr<PalInfo> pal) {
  pImpl->pallist.push_back(pal);
  pal->setOnline(true);
  emitNewPalOnline(pal);
}

void CoreThread::emitNewPalOnline(PPalInfo palInfo) {
  emitEvent(make_shared<NewPalOnlineEvent>(palInfo));
}

void CoreThread::emitNewPalOnline(const PalKey& palKey) {
  auto palInfo = GetPal(palKey);
  if (palInfo) {
    NewPalOnlineEvent event(palInfo);
    emitEvent(make_shared<NewPalOnlineEvent>(palInfo));
  } else {
    LOG_ERROR("emitNewPalOnline meet a unknown key: %s",
              palKey.ToString().c_str());
  }
}

void CoreThread::emitEvent(shared_ptr<const Event> event) {
  lock_guard<std::mutex> l(pImpl->waitingEventsMutex);
  pImpl->waitingEvents.push_back(event);
  this->pImpl->eventCount++;
  this->pImpl->lastEvent = event;
  signalEvent.emit(event);
}

/**
 * 向好友发送iptux特有的数据.
 * @param pal class PalInfo
 */
bool CoreThread::sendFeatureData(PPalInfo pal) noexcept {
  Command cmd(*this);
  char path[MAX_PATHLEN];
  const gchar* env;

  if (!programData->sign.empty()) {
    cmd.SendMySign(getUdpSock(), pal);
  }
  env = g_get_user_config_dir();
  snprintf(path, MAX_PATHLEN, "%s" ICON_PATH "/%s", env,
           programData->myicon.c_str());
  if (access(path, F_OK) == 0) {
    ifstream ifs(path);
    cmd.SendMyIcon(getUdpSock(), pal, ifs);
  }
  snprintf(path, MAX_PATHLEN, "%s" PHOTO_PATH "/photo", env);
  if (access(path, F_OK) == 0) {
    GError* error = nullptr;
    GSocket* sock = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_STREAM,
                                 G_SOCKET_PROTOCOL_TCP, &error);
    if (error != nullptr) {
      LOG_ERROR(_("Fatal Error!!\nFailed to create new socket!\n%s"),
                error->message);
      g_error_free(error);
      pImpl->lastErr = CORE_THREAD_ERR_SOCKET_CREATE_FAILED;
      return false;
    }

    bool ret = cmd.SendSublayer(sock, pal, IPTUX_PHOTOPICOPT, path);
    g_object_unref(sock);
    return ret;
  }
  return true;
}

void CoreThread::SendMyIcon(PPalInfo pal, istream& iss) {
  Command(*this).SendMyIcon(getUdpSock(), pal, iss);
}

void CoreThread::AddBlockIp(in_addr ipv4) {
  pImpl->blacklist =
      g_slist_append(pImpl->blacklist, GUINT_TO_POINTER(ipv4.s_addr));
}

bool CoreThread::SendMessage(CPPalInfo palInfo, const string& message) {
  Command cmd(*this);
  cmd.SendMessage(getUdpSock(), palInfo, message.c_str());
  return true;
}

bool CoreThread::SendMessage(CPPalInfo pal, const ChipData& chipData) {
  auto ptr = chipData.data.c_str();
  bool ret = true;

  switch (chipData.type) {
    case MessageContentType::STRING:
      /* 文本类型 */
      return SendMessage(pal, chipData.data);
    case MESSAGE_CONTENT_TYPE_PICTURE: {
      GError* error = nullptr;
      GSocket* sock = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_STREAM,
                                   G_SOCKET_PROTOCOL_TCP, &error);
      if (error != nullptr) {
        LOG_ERROR(_("Fatal Error!!\nFailed to create new socket!\n%s"),
                  error->message);
        g_error_free(error);
        return false;
      }
      ret = Command(*this).SendSublayer(sock, pal, IPTUX_MSGPICOPT, ptr);
      g_object_unref(sock);
      return ret;
    }
    default:
      g_assert_not_reached();
  }
}

bool CoreThread::SendMsgPara(shared_ptr<MsgPara> para) {
  for (int i = 0; i < int(para->dtlist.size()); ++i) {
    if (!SendMessage(para->getPal(), para->dtlist[i])) {
      LOG_ERROR("send message failed: %s", para->dtlist[i].ToString().c_str());
      return false;
    }
  }
  return true;
}

void CoreThread::AsyncSendMsgPara(std::shared_ptr<MsgPara> msgPara) {
  thread t(&CoreThread::SendMsgPara, this, msgPara);
  t.detach();
}

void CoreThread::InsertMessage(const MsgPara& para) {
  MsgPara para2 = para;
  this->emitEvent(make_shared<NewMessageEvent>(std::move(para2)));
}

void CoreThread::InsertMessage(MsgPara&& para) {
  this->emitEvent(make_shared<NewMessageEvent>(std::move(para)));
}

bool CoreThread::SendAskShared(PPalInfo pal) {
  Command(*this).SendAskShared(getUdpSock(), pal, 0, NULL);
  return true;
}

void CoreThread::SendSharedFiles(PPalInfo pal) {
  SendFile::SendSharedInfoEntry(this, pal);
}

void CoreThread::UpdateMyInfo() {
  Command cmd(*this);

  Lock();
  for (auto pal : pImpl->pallist) {
    if (pal->isOnline()) {
      cmd.SendAbsence(getUdpSock(), pal);
    }
    if (pal->isOnline() and pal->isCompatible()) {
      thread t1(bind(&CoreThread::sendFeatureData, this, _1), pal);
      t1.detach();
    }
  }
  Unlock();
  emitEvent(make_shared<const ConfigChangedEvent>());
}

/**
 * 发送通告本计算机下线的信息.
 * @param pal class PalInfo
 */
void CoreThread::SendBroadcastExit(PPalInfo pal) {
  Command cmd(*this);
  cmd.SendExit(getUdpSock(), pal);
}

int CoreThread::GetOnlineCount() const {
  if (this->started == false) {
    return 0;
  }

  int res = 0;
  for (auto pal : pImpl->pallist) {
    if (pal->isOnline()) {
      res++;
    }
  }
  return res;
}

void CoreThread::SendDetectPacket(const string& ipv4) {
  SendDetectPacket(inAddrFromString(ipv4));
}

void CoreThread::SendDetectPacket(in_addr ipv4) {
  Command(*this).SendDetectPacket(getUdpSock(), ipv4, port());
}

void CoreThread::emitSomeoneExit(const PalKey& palKey) {
  if (!GetPal(palKey)) {
    return;
  }
  DelPalFromList(palKey);
  emitEvent(make_shared<PalOfflineEvent>(palKey));
}

void CoreThread::EmitIconUpdate(const PalKey& palKey) {
  UpdatePalToList(palKey);
  emitEvent(make_shared<IconUpdateEvent>(palKey));
}

void CoreThread::SendExit(PPalInfo palInfo) {
  Command(*this).SendExit(getUdpSock(), palInfo);
}

const string& CoreThread::GetAccessPublicLimit() const {
  return programData->GetPasswd();
}

void CoreThread::SetAccessPublicLimit(const string& val) {
  programData->SetPasswd(val);
}

void CoreThread::AddPrivateFile(PFileInfo file) {
  g_assert(file);
  g_assert(file->fileid >= MAX_SHAREDFILE);
  g_assert(pImpl->privateFiles.count(file->fileid) == 0);
  pImpl->privateFiles[file->fileid] = file;
}

bool CoreThread::DelPrivateFile(uint32_t id) {
  return pImpl->privateFiles.erase(id) >= 1;
}

PFileInfo CoreThread::GetPrivateFileById(uint32_t id) {
  if (id < MAX_SHAREDFILE) {
    FileInfo* f = programData->GetShareFileInfo(id);
    if (!f)
      return PFileInfo();
    return make_shared<FileInfo>(*f);
  }

  auto res = pImpl->privateFiles.find(id);
  if (res == pImpl->privateFiles.end()) {
    return PFileInfo();
  }
  return res->second;
}

PFileInfo CoreThread::GetPrivateFileByPacketN(uint32_t packageNum,
                                              uint32_t filectime) {
  for (auto& i : pImpl->privateFiles) {
    if (i.second->packetn == packageNum && i.second->filenum == filectime) {
      return i.second;
    }
  }
  return PFileInfo();
}

void CoreThread::RegisterTransTask(std::shared_ptr<TransAbstract> task) {
  int taskId = ++(pImpl->lastTransTaskId);
  task->SetTaskId(taskId);
  pImpl->transTasks[taskId] = task;
  LOG_INFO("add trans task %d", taskId);
}

bool CoreThread::TerminateTransTask(int taskId) {
  auto task = pImpl->transTasks.find(taskId);
  if (task == pImpl->transTasks.end()) {
    return false;
  }
  task->second->TerminateTrans();
  return true;
}

void CoreThread::RecvFile(FileInfo* file) {
  auto rfdt = make_shared<RecvFileData>(this, file);
  RegisterTransTask(rfdt);
  rfdt->RecvFileDataEntry();
}

void CoreThread::RecvFileAsync(FileInfo* file) {
  thread t(&CoreThread::RecvFile, this, file);
  t.detach();
}

std::unique_ptr<TransFileModel> CoreThread::GetTransTaskStat(int taskId) const {
  auto task = pImpl->transTasks.find(taskId);
  if (task == pImpl->transTasks.end()) {
    return {};
  }
  return make_unique<TransFileModel>(task->second->getTransFileModel());
}

void CoreThread::clearFinishedTransTasks() {
  Lock();
  bool changed = false;
  for (auto it = pImpl->transTasks.begin(); it != pImpl->transTasks.end();) {
    if (it->second->getTransFileModel().isFinished()) {
      it = pImpl->transTasks.erase(it);
      changed = true;
    } else {
      it++;
    }
  }
  Unlock();

  if (changed) {
    emitEvent(make_shared<TransTasksChangedEvent>());
  }
}

bool CoreThread::SendAskSharedWithPassword(const PalKey& palKey,
                                           const std::string& password) {
  auto epasswd =
      g_base64_encode((const guchar*)(password.c_str()), password.size());
  Command(*this).SendAskShared(getUdpSock(), palKey, IPTUX_PASSWDOPT, epasswd);
  g_free(epasswd);
  return true;
}

void CoreThread::SendUnitMessage(const PalKey& palKey,
                                 uint32_t opttype,
                                 const string& message) {
  Command(*this).SendUnitMsg(getUdpSock(), GetPal(palKey), opttype,
                             message.c_str());
}

void CoreThread::SendGroupMessage(const PalKey& palKey,
                                  const std::string& message) {
  Command(*this).SendGroupMsg(getUdpSock(), GetPal(palKey), message.c_str());
}

void CoreThread::BcstFileInfoEntry(const vector<const PalInfo*>& pals,
                                   const vector<FileInfo*>& files) {
  SendFile::BcstFileInfoEntry(this, pals, files);
}

vector<unique_ptr<TransFileModel>> CoreThread::listTransTasks() const {
  vector<unique_ptr<TransFileModel>> res;
  Lock();
  for (auto it = pImpl->transTasks.begin(); it != pImpl->transTasks.end();
       it++) {
    res.push_back(make_unique<TransFileModel>(it->second->getTransFileModel()));
  }
  Unlock();
  return res;
}

int CoreThread::getEventCount() const {
  return this->pImpl->eventCount;
}

shared_ptr<const Event> CoreThread::getLastEvent() const {
  return this->pImpl->lastEvent;
}

PPalInfo CoreThread::getMe() {
  return this->pImpl->me;
}

string CoreThread::getUserIconPath() const {
  return stringFormat("%s%s", g_get_user_cache_dir(), ICON_PATH);
}

bool CoreThread::HasEvent() const {
  lock_guard<std::mutex> l(pImpl->waitingEventsMutex);
  return !this->pImpl->waitingEvents.empty();
}

shared_ptr<const Event> CoreThread::PopEvent() {
  lock_guard<std::mutex> l(pImpl->waitingEventsMutex);
  auto event = pImpl->waitingEvents.front();
  pImpl->waitingEvents.pop_front();
  return event;
}

enum CoreThreadErr CoreThread::getLastErr() const {
  return pImpl->lastErr;
}

}  // namespace iptux
