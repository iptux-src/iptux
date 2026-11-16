#!/usr/bin/env python3
"""A CLI iptux client based on the Iptux gobject-introspection bindings.

Useful for testing iptux without a GUI, e.g. two instances on the loopback
interface exchanging messages:

    python3 iptux-client.py --bind 127.0.0.1 --detect 127.0.0.2 \
        --send 127.0.0.2:hello --wait 5
    python3 iptux-client.py --bind 127.0.0.2 --send 127.0.0.1:hi --wait 5

Events received from the network are printed to stdout as lines:

    PAL-ONLINE <ipv4>
    MESSAGE <ipv4> <content>
"""

import argparse
import sys

import gi

gi.require_version('Iptux', '0.10')
gi.require_version('GLib', '2.0')
from gi.repository import Iptux
from gi.repository import GLib


def build_config(args):
    config = Iptux.Config.new_from_string('{}')
    if args.bind:
        config.set_string('bind_ip', args.bind)
    if args.port != 2425:
        config.set_int('port', args.port)
    return config


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--bind', default=None,
                        help='IP address to bind, default: all interfaces')
    parser.add_argument('--port', type=int, default=2425,
                        help='port to bind, default: 2425')
    parser.add_argument('--detect', action='append', default=[],
                        metavar='IP',
                        help='send a detect packet to IP (repeatable)')
    parser.add_argument('--send', action='append', default=[],
                        metavar='IP:MESSAGE',
                        help='send MESSAGE to IP (repeatable)')
    parser.add_argument('--wait', type=float, default=5.0,
                        help='seconds to keep running, default: 5')
    args = parser.parse_args()

    config = build_config(args)
    service = Iptux.Service.new(config)

    def on_pal_online(_service, ipv4):
        print(f'PAL-ONLINE {ipv4}', flush=True)

    def on_message_received(_service, ipv4, content):
        print(f'MESSAGE {ipv4} {content}', flush=True)

    service.connect('pal-online', on_pal_online)
    service.connect('message-received', on_message_received)
    service.set_log_level(GLib.LogLevelFlags.LEVEL_WARNING)

    loop = GLib.MainLoop()
    if not service.start():
        print('failed to start service', file=sys.stderr, flush=True)
        sys.exit(1)

    def do_actions():
        for ipv4 in args.detect:
            service.send_detect_packet(ipv4)
        return GLib.SOURCE_REMOVE

    def do_sends():
        for item in args.send:
            ipv4, _, message = item.partition(':')
            if not message:
                print(f'bad --send value: {item!r}', file=sys.stderr)
                continue
            ok = service.send_message_to(ipv4, message)
            if not ok:
                print(f'failed to send message to {ipv4}', file=sys.stderr,
                      flush=True)
        return GLib.SOURCE_REMOVE

    def quit():
        loop.quit()
        return GLib.SOURCE_REMOVE

    # wait a moment for the core thread to start, then act
    GLib.timeout_add(1000, do_actions)
    GLib.timeout_add(3000, do_sends)
    GLib.timeout_add(int((args.wait + 1) * 1000), quit)

    loop.run()
    service.stop()


if __name__ == '__main__':
    main()
