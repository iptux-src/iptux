#include "iptux-config.h"
#include "iptux-core/IptuxConfig.h"


struct _IptuxConfig {
  GObject parent_instance;
  iptux::IptuxConfig::Ptr config;
};

G_DEFINE_TYPE(IptuxConfig, iptux_config, G_TYPE_OBJECT)

static void iptux_config_class_init(IptuxConfigClass* //klass
    ) {
}

static void iptux_config_init(IptuxConfig* self) {
  self->config = nullptr;
}


extern "C" {

IptuxConfig* iptux_config_new(void) {
  return IPTUX_CONFIG(g_object_new(IPTUX_TYPE_CONFIG, nullptr));
}

}
