#!/usr/bin/env python3
"""End to end test: one iptux client sends a file to another over loopback.

Starts a sender client on 127.0.0.1 and a receiver client on 127.0.0.2
concurrently. The sender announces a regular file, the receiver accepts
it automatically (via --save-dir), and afterwards the received file is
compared with the original byte for byte.

Note: on macOS 127.0.0.2 requires `sudo ifconfig lo0 alias 127.0.0.2 up`.
"""

import os
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
CLIENT = os.path.join(HERE, 'iptux-client.py')

CONTENT = b'iptux file transfer e2e test\n' + os.urandom(4096) + b'\nEND\n'
WAIT = 10


def start_client(args):
    cmd = [sys.executable, CLIENT] + args
    return subprocess.Popen(cmd, stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE, text=True)


def main():
    tmpdir = tempfile.mkdtemp(prefix='iptux-file-test-')
    srcfile = os.path.join(tmpdir, 'src')
    with open(srcfile, 'wb') as f:
        f.write(CONTENT)
    savedir = os.path.join(tmpdir, 'received')
    os.mkdir(savedir)

    sender = start_client([
        '--bind', '127.0.0.1',
        '--detect', '127.0.0.2',
        '--send-file', f'127.0.0.2:{srcfile}',
        '--wait', str(WAIT),
    ])
    receiver = start_client([
        '--bind', '127.0.0.2',
        '--save-dir', savedir,
        '--wait', str(WAIT),
    ])

    try:
        out_send, _ = sender.communicate(timeout=WAIT + 60)
        out_recv, _ = receiver.communicate(timeout=WAIT + 60)
    except subprocess.TimeoutExpired:
        sender.kill()
        receiver.kill()
        print('FAIL: clients did not exit in time', file=sys.stderr)
        sys.exit(1)

    ok = True
    if f'FILE-SENT 127.0.0.2 {srcfile}' not in out_send:
        print(f'FAIL: sender did not announce the file\n{out_send}',
              file=sys.stderr)
        ok = False
    if f'FILE-RECEIVED 127.0.0.1 src {len(CONTENT)} 0' not in out_recv:
        print(f'FAIL: receiver did not receive the announcement\n{out_recv}',
              file=sys.stderr)
        ok = False

    received = os.path.join(savedir, 'src')
    deadline = time.time() + WAIT
    while not os.path.exists(received) and time.time() < deadline:
        time.sleep(0.2)
    if not os.path.exists(received):
        print(f'FAIL: file did not arrive at {received}\n{out_recv}',
              file=sys.stderr)
        ok = False
    else:
        with open(received, 'rb') as f:
            data = f.read()
        if data != CONTENT:
            print(f'FAIL: received file content differs: {data[:100]!r}',
                  file=sys.stderr)
            ok = False

    if ok:
        print('PASS: file was transferred and matches the original')
        print(out_send, end='')
        print(out_recv, end='')
        sys.exit(0)
    sys.exit(1)


if __name__ == '__main__':
    main()
