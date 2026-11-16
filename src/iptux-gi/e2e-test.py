#!/usr/bin/env python3
"""End to end test: two iptux python clients exchange messages over loopback.

Starts one client on 127.0.0.1 and one on 127.0.0.2 concurrently, then
checks that each receives the message sent by the other.

Note: on macOS 127.0.0.2 requires `sudo ifconfig lo0 alias 127.0.0.2 up`.
"""

import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
CLIENT = os.path.join(HERE, 'iptux-client.py')


def start_client(args):
    cmd = [sys.executable, CLIENT] + args
    return subprocess.Popen(cmd, stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE, text=True)


def main():
    client_a = start_client([
        '--bind', '127.0.0.1',
        '--detect', '127.0.0.2',
        '--send', '127.0.0.2:hello-from-A',
        '--wait', '6',
    ])
    client_b = start_client([
        '--bind', '127.0.0.2',
        '--send', '127.0.0.1:hello-from-B',
        '--wait', '6',
    ])
    try:
        out_a, _ = client_a.communicate(timeout=60)
        out_b, _ = client_b.communicate(timeout=60)
    except subprocess.TimeoutExpired:
        client_a.kill()
        client_b.kill()
        print('FAIL: clients did not exit in time', file=sys.stderr)
        sys.exit(1)

    ok = True
    if 'MESSAGE 127.0.0.2 hello-from-B' not in out_a:
        print(f'FAIL: client A did not receive the message from B\n{out_a}',
              file=sys.stderr)
        ok = False
    if 'MESSAGE 127.0.0.1 hello-from-A' not in out_b:
        print(f'FAIL: client B did not receive the message from A\n{out_b}',
              file=sys.stderr)
        ok = False
    if ok:
        print('PASS: both clients received the message from each other')
        print(out_a, end='')
        print(out_b, end='')
        sys.exit(0)
    sys.exit(1)


if __name__ == '__main__':
    main()
