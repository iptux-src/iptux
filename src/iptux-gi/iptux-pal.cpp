#include "iptux-pal.h"
#include "iptux-priv.h"

G_BEGIN_DECLS

G_DEFINE_TYPE(IptuxPal, iptux_pal, G_TYPE_OBJECT)

static void iptux_pal_class_init(IptuxPalClass* //klass
    ) {
}

static void iptux_pal_init(IptuxPal* self) {
  self->pal_info = nullptr;
}

char* iptux_pal_to_string(IptuxPal* self) {
  g_return_val_if_fail(self && self->pal_info, nullptr);

  std::string pal_str = self->pal_info->toString();
  return g_strdup(pal_str.c_str());
}

G_END_DECLS
