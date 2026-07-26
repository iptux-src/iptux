#include "iptux-service.h"
#include "iptux-core/CoreThread.h"

using namespace iptux;

struct _IptuxService {
  GObject parent_instance;
  CoreThread::Ptr core_thread;
};

G_DEFINE_TYPE(IptuxService, iptux_service, G_TYPE_OBJECT)

static void iptux_service_class_init(IptuxServiceClass* //klass
    ) {
}

static void iptux_service_init(IptuxService* self) {
  self->core_thread = 0;
}


extern "C" {

IptuxService* iptux_service_new(void) {
  return IPTUX_SERVICE(g_object_new(IPTUX_TYPE_SERVICE, nullptr));
}

}
