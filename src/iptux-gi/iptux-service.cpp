#include "iptux-service.h"
#include "iptux-core/CoreThread.h"
#include "iptux-pal.h"
#include "iptux-priv.h"
#include "iptux-utils/output.h"

using namespace iptux;

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
  CoreThread::Ptr core_thread;
  ::IptuxConfig* config;
};

G_DEFINE_TYPE(IptuxService, iptux_service, G_TYPE_OBJECT)

static void iptux_service_set_property(GObject* object,
                                       guint property_id,
                                       const GValue* value,
                                       GParamSpec* pspec) {
  IptuxService* self = IPTUX_SERVICE(object);

  switch (property_id) {
    case PROP_CONFIG:
      self->config = static_cast<::IptuxConfig*>(g_value_get_pointer(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
  }
}

static void iptux_service_constructed(GObject* object) {
  IptuxService* self = IPTUX_SERVICE(object);

  // 必须先调用父类的 constructed
  G_OBJECT_CLASS(iptux_service_parent_class)->constructed(object);

  // 💡 关键点：此时 self->config 已经被成功赋值，可以在这里使用它！
  if (self->config) {
    self->core_thread = std::make_shared<CoreThread>(self->config->config);
  }
}

static void iptux_service_class_init(IptuxServiceClass* klass) {
  GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

  gobject_class->set_property = iptux_service_set_property;
  gobject_class->constructed = iptux_service_constructed;

  obj_properties[PROP_CONFIG] = g_param_spec_pointer(
      "config", "Config", "IptuxConfig instance",
      static_cast<GParamFlags>(G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_properties(gobject_class, N_PROPERTIES,
                                    obj_properties);
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

  return self->core_thread->start();
}

bool iptux_service_stop(IptuxService* self) {
  if (!self || !self->core_thread) {
    g_warning("IptuxService or core_thread is null");
    return false;
  }

  self->core_thread->stop();
  return true;
}

GArray* iptux_service_get_pals(IptuxService* self) {
  if (!self || !self->core_thread) {
    g_warning("IptuxService or core_thread is null");
    return nullptr;
  }

  GArray* garray = g_array_new(FALSE, FALSE, sizeof(IptuxPal*));

  self->core_thread->OnlineForEach([&](PPalInfo pal_info) {
    IptuxPal* pal = IPTUX_PAL(g_object_new(IPTUX_TYPE_PAL, nullptr));
    pal->pal_info = pal_info;
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

  self->core_thread->SendMessage(pal->pal_info, std::string(message));
  return TRUE;
}

void iptux_service_send_message_async(IptuxService* self,
                                      IptuxPal* pal,
                                      const gchar* message,
                                      GCancellable*,
                                      GAsyncReadyCallback callback,
                                      gpointer user_data) {
  MsgPara::Ptr msgPara = std::make_shared<MsgPara>(pal->pal_info);

  g_return_if_fail(self != nullptr && self->core_thread != nullptr &&
                   pal != nullptr && message != nullptr);

  msgPara->dtlist.emplace_back(ChipData(std::string(message)));
  self->core_thread->AsyncSendMsgPara(msgPara);

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
