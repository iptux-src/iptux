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

/**
 * iptux_config_new_from_string:
 * @str: a JSON document
 *
 * Creates a new #IptuxConfig from a JSON string.
 *
 * Returns: (transfer full): a new #IptuxConfig
 */
IptuxConfig* iptux_config_new_from_string(const char* str) {
  return iptux_config_new_from_fname(str);
}

/**
 * iptux_config_set_string:
 * @self: an #IptuxConfig
 * @key: the config key
 * @value: the value to set
 */
void iptux_config_set_string(IptuxConfig* self, const char* key,
                             const char* value) {
  g_return_if_fail(self != nullptr && self->config != nullptr && key != nullptr);
  (**self->config).SetString(key, value ? value : "");
}

/**
 * iptux_config_set_int:
 * @self: an #IptuxConfig
 * @key: the config key
 * @value: the value to set
 */
void iptux_config_set_int(IptuxConfig* self, const char* key, int value) {
  g_return_if_fail(self != nullptr && self->config != nullptr && key != nullptr);
  (**self->config).SetInt(key, value);
}

/**
 * iptux_config_get_string:
 * @self: an #IptuxConfig
 * @key: the config key
 *
 * Returns: (transfer full): the value of @key, or an empty string if
 * the key is not set
 */
char* iptux_config_get_string(IptuxConfig* self, const char* key) {
  g_return_val_if_fail(self != nullptr && self->config != nullptr &&
                           key != nullptr,
                       nullptr);
  std::string value = (**self->config).GetString(key);
  return g_strdup(value.c_str());
}

G_END_DECLS
