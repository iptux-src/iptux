#include "iptux-service.h"
#include "iptux-core/Const.h"
#include "iptux-core/CoreThread.h"
#include "iptux-core/Event.h"
#include <atomic>
#include <memory>
#include <sys/stat.h>
#include <vector>
#include "iptux-pal.h"
#include "iptux-priv.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"

#ifdef SendMessage
#undef SendMessage
#endif

using namespace iptux;

G_BEGIN_DECLS

enum {
  PROP_0,
  PROP_CONFIG,
  N_PROPERTIES,
};

static GParamSpec* obj_properties[N_PROPERTIES] = {
    nullptr,
};

struct _IptuxService {
  GObject parent_instance;
  CoreThread::Ptr* core_thread;
  ::IptuxConfig* config;
  // copies of the file infos announced by pals, in arrival order;
  // iptux_service_recv_file() accepts one of them by index
  std::vector<iptux::PFileInfo>* received_files;
};

enum {
  SIGNAL_PAL_ONLINE,
  SIGNAL_MESSAGE_RECEIVED,
  SIGNAL_FILE_RECEIVED,
  SIGNAL_SEND_FILE_STARTED,
  SIGNAL_SEND_FILE_FINISHED,
  SIGNAL_RECV_FILE_STARTED,
  SIGNAL_RECV_FILE_FINISHED,
  N_SIGNALS,
};

static guint service_signals[N_SIGNALS] = {0, 0};

G_DEFINE_TYPE(IptuxService, iptux_service, G_TYPE_OBJECT)

static void iptux_service_set_property(GObject* object,
                                       guint property_id,
                                       const GValue* value,
                                       GParamSpec* pspec) {
  IptuxService* self = IPTUX_SERVICE(object);

  switch (property_id) {
    case PROP_CONFIG:
      self->config = static_cast<::IptuxConfig*>(g_value_get_pointer(value));
      g_object_ref(self->config);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
  }
}

static std::string chipDataContent(const iptux::ChipData& chip) {
  if (chip.type == iptux::MessageContentType::STRING) {
    return chip.data;
  }
  return chip.getSummary();
}

static void emitCoreEvent(IptuxService* self,
                          std::shared_ptr<const iptux::Event> event) {
  switch (event->getType()) {
    case iptux::EventType::NEW_PAL_ONLINE: {
      auto palEvent =
          std::dynamic_pointer_cast<const iptux::NewPalOnlineEvent>(event);
      if (!palEvent) {
        return;
      }
      std::string ipv4 = iptux::inAddrToString(palEvent->getPalInfo()->ipv4());
      g_signal_emit(self, service_signals[SIGNAL_PAL_ONLINE], 0,
                    ipv4.c_str());
      break;
    }
    case iptux::EventType::NEW_MESSAGE: {
      auto messageEvent =
          std::dynamic_pointer_cast<const iptux::NewMessageEvent>(event);
      if (!messageEvent) {
        return;
      }
      const iptux::MsgPara& para = messageEvent->getMsgPara();
      std::string ipv4 = iptux::inAddrToString(para.getPal()->ipv4());
      std::string content;
      for (const auto& chip : para.dtlist) {
        if (!content.empty()) {
          content += "\n";
        }
        content += chipDataContent(chip);
      }
      g_signal_emit(self, service_signals[SIGNAL_MESSAGE_RECEIVED], 0,
                    ipv4.c_str(), content.c_str());
      break;
    }
    case iptux::EventType::NEW_SHARE_FILE_FROM_FRIEND: {
      auto fileEvent =
          std::dynamic_pointer_cast<const iptux::NewShareFileFromFriendEvent>(
              event);
      if (!fileEvent) {
        return;
      }
      // keep a copy alive so it can be accepted later by index
      auto file = std::make_shared<iptux::FileInfo>(fileEvent->GetFileInfo());
      self->received_files->push_back(file);
      std::string ipv4 =
          iptux::inAddrToString(fileEvent->GetPalKey().GetIpv4());
      char* name = ipmsg_get_filename_me(file->filepath, nullptr);
      g_signal_emit(self, service_signals[SIGNAL_FILE_RECEIVED], 0,
                    ipv4.c_str(), name, (guint64)file->filesize,
                    (guint)(self->received_files->size() - 1));
      g_free(name);
      break;
    }
    case iptux::EventType::SEND_FILE_STARTED:
    case iptux::EventType::SEND_FILE_FINISHED:
    case iptux::EventType::RECV_FILE_STARTED:
    case iptux::EventType::RECV_FILE_FINISHED: {
      auto taskEvent =
          std::dynamic_pointer_cast<const iptux::AbstractTaskIdEvent>(event);
      if (!taskEvent) {
        return;
      }
      guint signal_id;
      switch (event->getType()) {
        case iptux::EventType::SEND_FILE_STARTED:
          signal_id = SIGNAL_SEND_FILE_STARTED;
          break;
        case iptux::EventType::SEND_FILE_FINISHED:
          signal_id = SIGNAL_SEND_FILE_FINISHED;
          break;
        case iptux::EventType::RECV_FILE_STARTED:
          signal_id = SIGNAL_RECV_FILE_STARTED;
          break;
        default:
          signal_id = SIGNAL_RECV_FILE_FINISHED;
          break;
      }
      g_signal_emit(self, service_signals[signal_id], 0,
                    (guint)taskEvent->GetTaskId());
      break;
    }
    default:
      break;
  }
}

static void iptux_service_constructed(GObject* object) {
  IptuxService* self = IPTUX_SERVICE(object);

  G_OBJECT_CLASS(iptux_service_parent_class)->constructed(object);

  if (self->config) {
    self->core_thread =
        new CoreThread::Ptr(CoreThread::create(*self->config->config));

    auto& core_thread = *(self->core_thread);
    core_thread->signalEvent.connect(
        [self](std::shared_ptr<const iptux::Event> event) {
          emitCoreEvent(self, event);
        });
  }
}

static void iptux_service_class_init(IptuxServiceClass* klass) {
  GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

  gobject_class->set_property = iptux_service_set_property;
  gobject_class->constructed = iptux_service_constructed;
  gobject_class->finalize = [](GObject* object) {
    IptuxService* self = IPTUX_SERVICE(object);
    // destroying the CoreThread stops and joins its threads, so no
    // more events can be emitted on a finalized service; it must be
    // destroyed before received_files, whose FileInfos may still be
    // referenced by a running transfer
    if (self->core_thread) {
      delete self->core_thread;
      self->core_thread = nullptr;
    }
    if (self->received_files) {
      delete self->received_files;
      self->received_files = nullptr;
    }
    if (self->config) {
      g_object_unref(self->config);
      self->config = nullptr;
    }
    G_OBJECT_CLASS(iptux_service_parent_class)->finalize(object);
  };

  obj_properties[PROP_CONFIG] = g_param_spec_pointer(
      "config", "Config", "IptuxConfig instance",
      static_cast<GParamFlags>(G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_properties(gobject_class, N_PROPERTIES,
                                    obj_properties);

  /**
   * IptuxService::pal-online:
   * @self: the #IptuxService that received the event
   * @ipv4: the IPv4 address of the pal that came online
   *
   * Emitted when a pal comes online.
   */
  service_signals[SIGNAL_PAL_ONLINE] =
      g_signal_new("pal-online", G_TYPE_FROM_CLASS(gobject_class),
                   G_SIGNAL_RUN_LAST, 0, nullptr, nullptr,
                   g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1,
                   G_TYPE_STRING);

  /**
   * IptuxService::message-received:
   * @self: the #IptuxService that received the message
   * @ipv4: the IPv4 address of the sender
   * @content: the message content
   *
   * Emitted when a text message is received.
   */
  service_signals[SIGNAL_MESSAGE_RECEIVED] =
      g_signal_new("message-received", G_TYPE_FROM_CLASS(gobject_class),
                   G_SIGNAL_RUN_LAST, 0, nullptr, nullptr,
                   nullptr, G_TYPE_NONE, 2,
                   G_TYPE_STRING, G_TYPE_STRING);

  /**
   * IptuxService::file-received:
   * @self: the #IptuxService that received the announcement
   * @ipv4: the IPv4 address of the sender
   * @filename: the base name of the announced file
   * @filesize: the size of the announced file
   * @index: the index to pass to iptux_service_recv_file() to accept it
   *
   * Emitted when a pal announces a file (or directory) for transfer.
   * Accept it with iptux_service_recv_file() to start the download.
   */
  service_signals[SIGNAL_FILE_RECEIVED] =
      g_signal_new("file-received", G_TYPE_FROM_CLASS(gobject_class),
                   G_SIGNAL_RUN_LAST, 0, nullptr, nullptr,
                   nullptr, G_TYPE_NONE, 4,
                   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT64, G_TYPE_UINT);

  /**
   * IptuxService::send-file-started / send-file-finished /
   * recv-file-started / recv-file-finished:
   * @self: the #IptuxService running the transfer
   * @taskid: the id of the transfer task
   *
   * Emitted when a file transfer task starts or finishes, on both the
   * sending and the receiving side.
   */
  service_signals[SIGNAL_SEND_FILE_STARTED] =
      g_signal_new("send-file-started", G_TYPE_FROM_CLASS(gobject_class),
                   G_SIGNAL_RUN_LAST, 0, nullptr, nullptr,
                   nullptr, G_TYPE_NONE, 1, G_TYPE_UINT);
  service_signals[SIGNAL_SEND_FILE_FINISHED] =
      g_signal_new("send-file-finished", G_TYPE_FROM_CLASS(gobject_class),
                   G_SIGNAL_RUN_LAST, 0, nullptr, nullptr,
                   nullptr, G_TYPE_NONE, 1, G_TYPE_UINT);
  service_signals[SIGNAL_RECV_FILE_STARTED] =
      g_signal_new("recv-file-started", G_TYPE_FROM_CLASS(gobject_class),
                   G_SIGNAL_RUN_LAST, 0, nullptr, nullptr,
                   nullptr, G_TYPE_NONE, 1, G_TYPE_UINT);
  service_signals[SIGNAL_RECV_FILE_FINISHED] =
      g_signal_new("recv-file-finished", G_TYPE_FROM_CLASS(gobject_class),
                   G_SIGNAL_RUN_LAST, 0, nullptr, nullptr,
                   nullptr, G_TYPE_NONE, 1, G_TYPE_UINT);
}

static void iptux_service_init(IptuxService* self) {
  self->core_thread = 0;
  self->received_files = new std::vector<iptux::PFileInfo>();
}

IptuxService* iptux_service_new(::IptuxConfig* config) {
  return IPTUX_SERVICE(
      g_object_new(IPTUX_TYPE_SERVICE, "config", config, nullptr));
}

bool iptux_service_start(IptuxService* self) {
  if (!self || !self->core_thread) {
    g_warning("IptuxService or core_thread is null");
    return false;
  }

  auto& core_thread = *(self->core_thread);

  return core_thread->start();
}

bool iptux_service_stop(IptuxService* self) {
  if (!self || !self->core_thread) {
    g_warning("IptuxService or core_thread is null");
    return false;
  }

  auto& core_thread = *(self->core_thread);

  core_thread->stop();
  return true;
}

GArray* iptux_service_get_pals(IptuxService* self) {
  if (!self || !self->core_thread) {
    g_warning("IptuxService or core_thread is null");
    return nullptr;
  }

  auto& core_thread = *(self->core_thread);

  GArray* garray = g_array_new(FALSE, FALSE, sizeof(IptuxPal*));

  core_thread->OnlineForEach([&](PalInfo::Ptr pal_info) {
    IptuxPal* pal = IPTUX_PAL(g_object_new(IPTUX_TYPE_PAL, nullptr));
    pal->pal_info = new PalInfo::Ptr(pal_info);
    g_array_append_val(garray, pal);
    return true;  // Continue iteration
  });

  return garray;
}

gboolean iptux_service_send_message(IptuxService* self,
                                    IptuxPal* pal,
                                    const gchar* message,
                                    GError**) {
  g_return_val_if_fail(self != nullptr && self->core_thread != nullptr &&
                           pal != nullptr && message != nullptr,
                       FALSE);
  auto& core_thread = *(self->core_thread);
  core_thread->SendMessage(*pal->pal_info, std::string(message));
  return TRUE;
}

gboolean iptux_service_send_message_to(IptuxService* self,
                                       const gchar* ipv4,
                                       const gchar* message,
                                       GError** error) {
  g_return_val_if_fail(self != nullptr && self->core_thread != nullptr &&
                           ipv4 != nullptr && message != nullptr,
                       FALSE);
  auto& core_thread = *(self->core_thread);

  auto pal = core_thread->GetPal(std::string(ipv4));
  if (!pal) {
    if (error) {
      *error = g_error_new(g_quark_from_static_string("iptux-service"), 1,
                           "pal %s not found (is it online?)", ipv4);
    }
    return FALSE;
  }
  core_thread->SendMessage(pal, std::string(message));
  return TRUE;
}

void iptux_service_send_detect_packet(IptuxService* self, const gchar* ipv4) {
  g_return_if_fail(self != nullptr && self->core_thread != nullptr &&
                   ipv4 != nullptr);
  auto& core_thread = *(self->core_thread);
  core_thread->SendDetectPacket(std::string(ipv4));
}

// File ids must not collide with the ids used by the (GUI-internal)
// shared-file registry; ids below MAX_SHAREDFILE are reserved for the
// iptux protocol's built-in shared files.
static std::atomic<uint32_t> nextFileId{MAX_SHAREDFILE};

static gboolean set_service_error(GError** error, const gchar* format, ...) {
  if (error) {
    va_list args;
    va_start(args, format);
    *error = g_error_new_valist(g_quark_from_static_string("iptux-service"), 1,
                                format, args);
    va_end(args);
  }
  return FALSE;
}

gboolean iptux_service_send_file_to(IptuxService* self,
                                    const gchar* ipv4,
                                    const gchar* path,
                                    GError** error) {
  g_return_val_if_fail(self != nullptr && self->core_thread != nullptr &&
                           ipv4 != nullptr && path != nullptr,
                       FALSE);
  auto& core_thread = *(self->core_thread);

  auto pal = core_thread->GetPal(std::string(ipv4));
  if (!pal) {
    return set_service_error(error, "pal %s not found (is it online?)", ipv4);
  }

  struct stat st;
  if (stat(path, &st) == -1 ||
      !(S_ISREG(st.st_mode) || S_ISDIR(st.st_mode))) {
    return set_service_error(error, "cannot send file %s: no such regular "
                                    "file or directory", path);
  }

  auto file = std::make_shared<iptux::FileInfo>();
  file->fileid = nextFileId.fetch_add(1);
  file->fileattr =
      S_ISREG(st.st_mode) ? iptux::FileAttr::REGULAR : iptux::FileAttr::DIRECTORY;
  file->filesize = iptux::utils::fileOrDirectorySize(path);
  file->filepath = g_strdup(path);
  file->filectime = uint32_t(st.st_ctime);
  file->filemtime = uint32_t(st.st_mtime);
  file->filenum = 0;
  file->fileown = pal;

  core_thread->Lock();
  core_thread->AddPrivateFile(file);
  core_thread->Unlock();

  std::vector<const PalInfo*> pals{pal.get()};
  std::vector<iptux::FileInfo*> files{file.get()};
  core_thread->BcstFileInfoEntry(pals, files);
  return TRUE;
}

gboolean iptux_service_recv_file(IptuxService* self,
                                 guint index,
                                 const gchar* save_dir,
                                 GError** error) {
  g_return_val_if_fail(self != nullptr && self->core_thread != nullptr &&
                           save_dir != nullptr,
                       FALSE);
  auto& core_thread = *(self->core_thread);

  if (!self->received_files || index >= self->received_files->size()) {
    return set_service_error(error, "no announced file with index %u", index);
  }

  auto file = (*self->received_files)[index];
  char* name = ipmsg_get_filename_me(file->filepath, nullptr);
  gchar* savepath = g_build_filename(save_dir, name, nullptr);
  g_free(name);
  g_free(file->filepath);
  file->filepath = savepath;

  core_thread->RecvFileAsync(file.get());
  return TRUE;
}

void iptux_service_send_message_async(IptuxService* self,
                                      IptuxPal* pal,
                                      const gchar* message,
                                      GCancellable*,
                                      GAsyncReadyCallback callback,
                                      gpointer user_data) {
  MsgPara::Ptr msgPara = std::make_shared<MsgPara>(*pal->pal_info);

  g_return_if_fail(self != nullptr && self->core_thread != nullptr &&
                   pal != nullptr && message != nullptr);

  auto core_thread = *(self->core_thread);

  msgPara->dtlist.emplace_back(ChipData(std::string(message)));
  core_thread->AsyncSendMsgPara(msgPara);

  callback(G_OBJECT(self), NULL,
           user_data);  // Notify that the operation is complete
  return;
}

gboolean iptux_service_send_message_finish(IptuxService*,
                                           GAsyncResult*,
                                           GError**) {
  return TRUE;
}

void iptux_service_set_log_level(IptuxService* self, GLogLevelFlags level) {
  g_return_if_fail(self != nullptr && self->core_thread != nullptr);

  Log::setLogLevel(static_cast<LogLevel>(level));
}

G_END_DECLS
