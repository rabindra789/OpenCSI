"""Monitor serial for TCP server ready, then start TCP traffic."""
import serial
import subprocess
import sys
import time
import os

PORT = "COM6"
BAUD = 115200
TARGET_IP = "192.168.29.140"
TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))

output = sys.argv[1]
duration = int(sys.argv[2]) if len(sys.argv) > 2 else 120

ser = serial.Serial(PORT, BAUD, timeout=1)
time.sleep(2)

# Watch serial for TCP server ready
start = time.time()
server_ready = False
capture_started = False

with open(output, "wb") as f:
    while time.time() - start < duration:
        data = ser.read(1024)
        if data:
            f.write(data)
            if not server_ready:
                text = data.decode('utf-8', errors='replace')
                if "Listening on port 8888" in text:
                    print("[runner] TCP server ready, starting traffic")
                    server_ready = True
                    # Start TCP client
                    cmd = [sys.executable, os.path.join(TOOLS_DIR, "tcp_stimulus.py"), "10", str(duration)]
                    subprocess.Popen(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

ser.close()
print(f"[runner] done: {output}")
