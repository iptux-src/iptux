#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define IPTUX_TYPE_CONFIG (iptux_config_get_type())
G_DECLARE_FINAL_TYPE(IptuxConfig, iptux_config, IPTUX, CONFIG, GObject)

IptuxConfig* iptux_config_new_from_fname(const char* fname);

IptuxConfig* iptux_config_new_from_string(const char* str);

void iptux_config_set_string(IptuxConfig* self, const char* key,
                             const char* value);

void iptux_config_set_int(IptuxConfig* self, const char* key, int value);

char* iptux_config_get_string(IptuxConfig* self, const char* key);

G_END_DECLS