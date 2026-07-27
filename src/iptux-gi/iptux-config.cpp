#include "iptux-config.h"
#include "iptux-core/IptuxConfig.h"
#include "iptux-priv.h"

G_BEGIN_DECLS

G_DEFINE_TYPE(IptuxConfig, iptux_config, G_TYPE_OBJECT)

static void iptux_config_class_init(IptuxConfigClass* klass) {
  GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->finalize = [](GObject* object) {
    IptuxConfig* self = IPTUX_CONFIG(object);
    if (self->config) {
      delete self->config;
      self->config = nullptr;
    }
    G_OBJECT_CLASS(iptux_config_parent_class)->finalize(object);
  };
}

static void iptux_config_init(IptuxConfig* self) {
  self->config = nullptr;
}

IptuxConfig* iptux_config_new_from_fname(const char* fname) {
  IptuxConfig* self = IPTUX_CONFIG(g_object_new(IPTUX_TYPE_CONFIG, nullptr));
  self->config =
      new iptux::IptuxConfig::Ptr(iptux::IptuxConfig::newFromString(fname));
  return self;
}

G_END_DECLS
