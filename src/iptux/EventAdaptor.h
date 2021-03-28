#ifndef IPTUX_EVENT_ADAPTOR_H
#define IPTUX_EVENT_ADAPTOR_H

#include <glib.h>
#include <sigc++/signal.h>

#include "iptux-core/Event.h"

namespace iptux {

using EventCallback = std::function<void(std::shared_ptr<const Event>)>;

class EventAdaptor : public sigc::trackable {
 public:
  EventAdaptor(sigc::signal<void(std::shared_ptr<const Event>)>& signal,
               EventCallback callback);

 private:
  void processEvent(std::shared_ptr<const Event> event);
  static gboolean processEventCallback(gpointer data);

 private:
  EventCallback callback;
};

}  // namespace iptux

#endif
