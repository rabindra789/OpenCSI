"""Run an EXP-004 scenario with serial capture and optional traffic stimulus.

Usage:
    python exp_runner.py <scenario> <output_file> [duration]

Scenarios:
    idle            No traffic
    ping-1s         1 ping per second (Windows ping)
    ping-continuous Continuous ping (Windows ping)
    udp-10          10 UDP packets/second (tools/udp_sender.py)
    udp-100         100 UDP packets/second (tools/udp_sender.py)
    tcp-download    TCP stream (tools/tcp_stimulus.py, ESP32 must have TCP server)
    idle-repeat     No traffic (post-stimulus baseline)
"""

import serial
import subprocess
import sys
import time
import threading
import os

PORT = "COM6"
BAUD = 115200
TARGET_IP = "192.168.29.140"
TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
DURATION_S = int(sys.argv[3]) if len(sys.argv) > 3 else 180

SCENARIOS = {
    "idle":             None,
    "idle-repeat":      None,
    "ping-1s":          ["ping", "-n", "120", TARGET_IP],
    "ping-continuous":  ["ping", "-t", TARGET_IP],
    "udp-10":           [sys.executable, os.path.join(TOOLS_DIR, "udp_sender.py"), "10", str(DURATION_S)],
    "udp-100":          [sys.executable, os.path.join(TOOLS_DIR, "udp_sender.py"), "100", str(DURATION_S)],
    "tcp-download":     [sys.executable, os.path.join(TOOLS_DIR, "tcp_stimulus.py"), "10", str(DURATION_S)],
}

def serial_capture(output_file, duration):
    ser = serial.Serial(PORT, BAUD, timeout=1)
    time.sleep(2)
    start = time.time()
    with open(output_file, "wb") as f:
        while time.time() - start < duration:
            data = ser.read(1024)
            if data:
                f.write(data)
    ser.close()
    print(f"[capture] written {output_file} ({duration}s)")

def run_traffic(proc):
    if proc is None:
        print("[traffic] no traffic for this scenario")
        return
    print(f"[traffic] starting: {' '.join(proc)}")
    p = subprocess.Popen(proc, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return p

def stop_traffic(proc):
    if proc is None:
        return
    print("[traffic] stopping...")
    proc.terminate()
    try:
        proc.wait(timeout=5)
    except subprocess.TimeoutExpired:
        proc.kill()

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)

    scenario = sys.argv[1]
    output = sys.argv[2]
    duration = DURATION_S

    if scenario not in SCENARIOS:
        print(f"Unknown scenario: {scenario}")
        print(f"Known: {', '.join(SCENARIOS.keys())}")
        sys.exit(1)

    cmd = SCENARIOS[scenario]

    print(f"=== EXP-004 RUNNER [{scenario}] ===")
    print(f"Capture: {output} ({duration}s)")
    print(f"Traffic: {cmd or 'none'}")

    # Start traffic process if needed
    traffic_proc = None
    if scenario == "ping-continuous":
        traffic_proc = subprocess.Popen(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    elif cmd:
        traffic_proc = subprocess.Popen(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    # Start serial capture in background thread
    capture_thread = threading.Thread(target=serial_capture, args=(output, duration), daemon=True)
    capture_thread.start()

    # Wait for capture to finish
    capture_thread.join()

    # Stop traffic
    if traffic_proc:
        if scenario == "ping-continuous":
            traffic_proc.terminate()
        else:
            traffic_proc.wait()
        stop_traffic(traffic_proc)

    print(f"=== EXP-004 RUNNER COMPLETE [{scenario}] ===")
