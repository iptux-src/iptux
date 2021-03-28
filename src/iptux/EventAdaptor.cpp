#include "config.h"
#include "EventAdaptor.h"

using namespace std;

namespace iptux {

class EventData {
 public:
  EventData(EventAdaptor* adaptor, shared_ptr<const Event> event) {
    this->adaptor = adaptor;
    this->event = event;
  }

  EventAdaptor* adaptor;
  shared_ptr<const Event> event;
};

EventAdaptor::EventAdaptor(
    sigc::signal<void(std::shared_ptr<const Event>)>& signalEvent,
    EventCallback callback)
    : callback(callback) {
  signalEvent.connect(sigc::mem_fun(*this, &EventAdaptor::processEvent));
}

void EventAdaptor::processEvent(std::shared_ptr<const Event> event) {
  EventData* callback = new EventData(this, event);
  g_idle_add(processEventCallback, callback);
}

gboolean EventAdaptor::processEventCallback(gpointer data) {
  EventData* ed = (EventData*)data;
  ed->adaptor->callback(ed->event);
  delete ed;
  return G_SOURCE_REMOVE;
}

}  // namespace iptux
