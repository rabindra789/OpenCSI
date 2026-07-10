"""Send TCP data to ESP32 at a controlled rate for EXP-004 TCP download scenario.

Connects to TCP server on ESP32 port 8888, sends data at given rate.
"""
import socket
import sys
import time

TARGET_IP = "192.168.29.140"
TARGET_PORT = 8888

def send_tcp_stream(rate_hz: float, duration_s: float):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(10)
    sock.connect((TARGET_IP, TARGET_PORT))
    payload = b"T" * 200
    interval = 1.0 / rate_hz
    deadline = time.time() + duration_s
    sent = 0
    next_tx = time.time()

    while time.time() < deadline:
        now = time.time()
        if now >= next_tx:
            try:
                sock.sendall(payload)
                sent += 1
            except Exception:
                break
            next_tx += interval
            if next_tx < now:
                next_tx = now + interval
        else:
            time.sleep(min(0.001, next_tx - now))

    sock.close()
    return sent

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <rate_hz> <duration_s>")
        sys.exit(1)
    rate = float(sys.argv[1])
    dur = float(sys.argv[2])
    print(f"Connecting to {TARGET_IP}:{TARGET_PORT}...")
    sent = send_tcp_stream(rate, dur)
    print(f"Done. Sent {sent} sends (target: {rate * dur:.0f})")
