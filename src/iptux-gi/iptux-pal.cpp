#include "iptux-pal.h"
#include "iptux-priv.h"



G_DEFINE_TYPE(IptuxPal, iptux_pal, G_TYPE_OBJECT)

static void iptux_pal_class_init(IptuxPalClass* //klass
    ) {
}

static void iptux_pal_init(IptuxPal* self) {
  self->pal_info = nullptr;
}


extern "C" {

}
