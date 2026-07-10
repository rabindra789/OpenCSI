"""Send ICMP echo requests at a controlled rate using raw sockets.

Usage: python ping_fast.py <target_ip> <rate_hz> <duration_s>
"""
import socket
import struct
import sys
import time
import os

TARGET_IP = "192.168.29.140"

def checksum(data):
    s = 0
    for i in range(0, len(data), 2):
        w = (data[i] << 8) + (data[i+1] if i+1 < len(data) else 0)
        s = (s + w) & 0xffff
    s = (s >> 16) + (s & 0xffff)
    return (~s) & 0xffff

def create_packet(seq):
    header = struct.pack("!BBHHH", 8, 0, 0, 0, seq)
    payload = b"CSITEST" + b"X" * (56 - 8)
    chk = checksum(header + payload)
    header = struct.pack("!BBHHH", 8, 0, chk, 0, seq)
    return header + payload

def send_pings(rate_hz, duration_s):
    interval = 1.0 / rate_hz
    deadline = time.time() + duration_s
    sent = 0

    sock = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.getprotobyname("icmp"))
    sock.settimeout(0.1)

    next_tx = time.time()
    while time.time() < deadline:
        now = time.time()
        if now >= next_tx:
            try:
                pkt = create_packet(sent & 0xffff)
                sock.sendto(pkt, (TARGET_IP, 1))
                sent += 1
            except Exception:
                pass
            next_tx += interval
            if next_tx < now:
                next_tx = now + interval
        else:
            time.sleep(min(0.001, next_tx - now))

    sock.close()
    return sent

if __name__ == "__main__":
    rate = float(sys.argv[1]) if len(sys.argv) > 1 else 10
    dur = float(sys.argv[2]) if len(sys.argv) > 2 else 120
    print(f"Sending ICMP to {TARGET_IP} at {rate} Hz for {dur}s...")
    sent = send_pings(rate, dur)
    print(f"Done. Sent {sent} pings")
