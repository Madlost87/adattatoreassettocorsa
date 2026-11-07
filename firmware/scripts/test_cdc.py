#!/usr/bin/env python3
"""No-dependency CDC command tool. Default command: ID?"""
import fcntl, os, select, struct, sys, termios, time

port = sys.argv[1] if len(sys.argv) >= 2 else "/dev/ttyACM0"
command = sys.argv[2] if len(sys.argv) >= 3 else "ID?"
fd = os.open(port, os.O_RDWR | os.O_NOCTTY)
try:
    cfg = termios.tcgetattr(fd)
    cfg[0] = cfg[1] = cfg[3] = 0
    cfg[2] |= termios.CLOCAL | termios.CREAD | termios.CS8
    cfg[4] = cfg[5] = termios.B115200
    termios.tcsetattr(fd, termios.TCSANOW, cfg)
    termios.tcflush(fd, termios.TCIOFLUSH)
    # CDC ACM boards often wait for the host to assert DTR/RTS before
    # accepting commands.  Do it explicitly instead of relying on a terminal.
    try:
        fcntl.ioctl(fd, termios.TIOCMBIS, struct.pack("I", termios.TIOCM_DTR | termios.TIOCM_RTS))
    except OSError:
        pass
    time.sleep(0.3)
    os.write(fd, command.encode("ascii") + b"\r\n")
    end = time.monotonic() + 3
    response = b""
    while time.monotonic() < end:
        ready, _, _ = select.select([fd], [], [], end - time.monotonic())
        if ready:
            response += os.read(fd, 128)
    print(response.decode("ascii", "replace").strip() or "Nessuna risposta CDC")
finally:
    os.close(fd)
