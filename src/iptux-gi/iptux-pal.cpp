#include "iptux-pal.h"
#include "iptux-priv.h"

G_BEGIN_DECLS

G_DEFINE_TYPE(IptuxPal, iptux_pal, G_TYPE_OBJECT)

static void iptux_pal_class_init(IptuxPalClass* klass) {
  GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->finalize = [](GObject* object) {
    IptuxPal* self = IPTUX_PAL(object);
    if (self->pal_info) {
      delete self->pal_info;
      self->pal_info = nullptr;
    }
    G_OBJECT_CLASS(iptux_pal_parent_class)->finalize(object);
  };
}

static void iptux_pal_init(IptuxPal* self) {
  self->pal_info = nullptr;
}

char* iptux_pal_to_string(IptuxPal* self) {
  g_return_val_if_fail(self && self->pal_info, nullptr);

  auto& pal_info = *(self->pal_info);

  std::string pal_str = pal_info->toString();
  return g_strdup(pal_str.c_str());
}

G_END_DECLS
