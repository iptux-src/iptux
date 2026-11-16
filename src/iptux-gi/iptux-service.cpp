#include "iptux-service.h"
#include "iptux-core/CoreThread.h"
#include "iptux-core/Event.h"
#include <memory>
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
};

enum {
  SIGNAL_PAL_ONLINE,
  SIGNAL_MESSAGE_RECEIVED,
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
    // more events can be emitted on a finalized service
    if (self->core_thread) {
      delete self->core_thread;
      self->core_thread = nullptr;
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
}

static void iptux_service_init(IptuxService* self) {
  self->core_thread = 0;
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
