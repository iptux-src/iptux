#include "config.h"
#include "CommandMode.h"

#include <glib/gi18n.h>

#include "iptux/ipmsg.h"
#include "iptux-core/utils.h"

using namespace std;

namespace iptux {
  string CommandMode::toString() const {
    switch(mode) {
    case IPMSG_BR_ENTRY:
      return "BR_ENTRY";
    case IPMSG_BR_EXIT:
      return "BR_EXIT";
    case IPMSG_ANSENTRY:
      return "ANSENTRY";
    case IPMSG_BR_ABSENCE:
      return "BR_ABSENCE";
    case IPMSG_SENDMSG:
      return "SENDMSG";
    case IPMSG_RECVMSG:
      return "RECVMSG";
    case IPTUX_ASKSHARED:
      return "ASKSHARED";
    case IPTUX_SENDICON:
      return "SENDICON";
    case IPTUX_SEND_SIGN:
      return "SEND_SIGN";
    case IPTUX_SENDMSG:
      return "SENDMSG";
    default:
      return stringFormat(_("unkown command mode: %d"), mode);
    }
  }
}
