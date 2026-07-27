#include "iptux-service.h"
#include "iptux-core/CoreThread.h"
#include "iptux-priv.h"

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


extern "C" {

IptuxService* iptux_service_new(::IptuxConfig* config) {
  return IPTUX_SERVICE(
      g_object_new(IPTUX_TYPE_SERVICE, "config", config, nullptr));
}
}
