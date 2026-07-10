import serial, sys, time

PORT = "COM6"
BAUD = 115200
DURATION_S = 120
OUTPUT = sys.argv[1] if len(sys.argv) > 1 else "capture-raw.txt"

if len(sys.argv) > 2:
    DURATION_S = int(sys.argv[2])

ser = serial.Serial(PORT, BAUD, timeout=1)
time.sleep(2)

start = time.time()
with open(OUTPUT, "wb") as f:
    while time.time() - start < DURATION_S + 10:
        data = ser.read(1024)
        if data:
            f.write(data)

ser.close()
print(f"Captured {OUTPUT} ({DURATION_S}s)")
