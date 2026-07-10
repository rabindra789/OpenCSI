"""Send UDP packets to ESP32 at a controlled rate for EXP-002."""
import socket
import time
import sys

TARGET_IP = "192.168.29.140"
TARGET_PORT = 9999

def send_burst(rate_hz: float, duration_s: float):
    """Send UDP packets at rate_hz for duration_s seconds."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    payload = b"CSITEST" + b"X" * 50  # small packet ~58 bytes
    interval = 1.0 / rate_hz
    deadline = time.time() + duration_s
    sent = 0
    next_tx = time.time()

    while time.time() < deadline:
        now = time.time()
        if now >= next_tx:
            try:
                sock.sendto(payload, (TARGET_IP, TARGET_PORT))
                sent += 1
            except Exception:
                pass
            next_tx += interval
            # if we fell behind, catch up (don't burst)
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
    print(f"Sending UDP to {TARGET_IP}:{TARGET_PORT} at {rate} Hz for {dur}s...")
    sent = send_burst(rate, dur)
    print(f"Done. Sent {sent} packets (target: {rate * dur:.0f})")
