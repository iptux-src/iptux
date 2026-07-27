#include "iptux-config.h"
#include "iptux-core/IptuxConfig.h"
#include "iptux-priv.h"

G_BEGIN_DECLS

G_DEFINE_TYPE(IptuxConfig, iptux_config, G_TYPE_OBJECT)

static void iptux_config_class_init(IptuxConfigClass* //klass
    ) {
}

static void iptux_config_init(IptuxConfig* self) {
  self->config = nullptr;
}

IptuxConfig* iptux_config_new_from_fname(const char* fname) {
  IptuxConfig* self = IPTUX_CONFIG(g_object_new(IPTUX_TYPE_CONFIG, nullptr));
  self->config = iptux::IptuxConfig::newFromString(fname);
  return self;
}

G_END_DECLS
