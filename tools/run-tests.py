#!/usr/bin/env python3

import subprocess
import time

def runQemu():
    qemu = 'qemu-system-i386'
    cmd = [qemu, '-monitor', 'stdio', '-serial', 'file:/dev/null', '-serial', 'file:kernel-x86.log', '-cdrom', 'bin/OS-x86.iso']

    proc = subprocess.Popen(cmd, stdin=subprocess.PIPE)
    time.sleep(3)

    proc.communicate(b'quit\n')
    proc.wait()

def main():
    runQemu()

if __name__ == '__main__':
    main()
