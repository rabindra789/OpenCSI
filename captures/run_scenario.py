import subprocess
import sys
import time
import os
import signal

SCENARIO = sys.argv[1] if len(sys.argv) > 1 else "idle"
DURATION = 70  # 60s experiment + 10s buffer
OUT_FILE = f"D:\\workspace\\development\\OpenCSI\\captures\\exp-001-{SCENARIO}-raw.txt"
PROJECT_DIR = "D:\\workspace\\development\\OpenCSI\\firmware\\m2_exp_001"

# Start monitor
with open(OUT_FILE, "w", encoding="utf-8") as f:
    monitor = subprocess.Popen(
        ["python", "-m", "esp_idf_monitor", "--disable-auto-color",
         "-p", "COM6", "-b", "115200", "build/exp_001.elf"],
        cwd=PROJECT_DIR,
        stdout=f,
        stderr=subprocess.STDOUT,
        env={**os.environ, "PYTHONIOENCODING": "utf-8"}
    )

    # Wait for connection
    connected = False
    start_wait = time.time()
    while time.time() - start_wait < 60:
        if os.path.exists(OUT_FILE):
            with open(OUT_FILE, "r", encoding="utf-8") as f2:
                content = f2.read()
                if "Connected. IP:" in content:
                    connected = True
                    print(f"ESP32 connected at {time.time() - start_wait:.0f}s")
                    break
        time.sleep(1)

    if not connected:
        print("WARNING: ESP32 did not connect within 60s")

    # Start ping to router
    if SCENARIO == "ping-router":
        ping = subprocess.Popen(["ping", "192.168.29.1", "-t"],
                                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        print("Ping to router started")
    elif SCENARIO == "ping-esp32":
        ping = subprocess.Popen(["ping", "192.168.29.140", "-t"],
                                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        print("Ping to ESP32 started")
    else:
        ping = None

    # Wait for experiment to complete
    elapsed = time.time() - start_wait
    remaining = max(0, DURATION - elapsed)
    print(f"Waiting {remaining:.0f}s more for experiment completion...")
    time.sleep(remaining)

    # Cleanup
    if ping:
        ping.terminate()
        ping.wait()
        print("Ping stopped")

    monitor.terminate()
    try:
        monitor.wait(timeout=5)
    except:
        monitor.kill()
    print(f"Monitor stopped. Output saved to {OUT_FILE}")
