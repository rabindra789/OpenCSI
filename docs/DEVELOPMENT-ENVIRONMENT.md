# Development Environment — OpenCSI

This document describes the minimal, reproducible Windows development environment
for OpenCSI. Every contributor should be able to follow these steps and arrive at
an identical toolchain.

## Philosophy

- **Stability over features** — use the same ESP-IDF version pinned here.
- **Reproducibility** — every step has a verification check.
- **Transparency** — understand what each command does.

## Prerequisites

| Item | Required | Check |
|------|----------|-------|
| Windows 11 | ✓ | `winver` |
| Python ≥ 3.8 | ✓ | `python --version` |
| Git ≥ 2.30 | ✓ | `git --version` |
| ESP32 DevKit V1 (USB connected) | ✓ | See Phase A |
| USB data cable (not charge-only) | ✓ | |

## Overview

```
Phase A — Preflight         Verify system readiness
Phase B — Install ESP-IDF   Download & configure toolchain
Phase C — Verify Toolchain  Confirm compiler & tools work
Phase D — Build & Flash     Test project on real hardware
Phase E — Persistence       Make the environment reusable
```

---

## Phase A — Preflight

### A1 — Enable Windows long path support

ESP-IDF has deeply nested directories. Without long paths, `install.ps1` will fail.

```powershell
# Run as Administrator (one-time):
New-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem" `
  -Name "LongPathsEnabled" -Value 1 -PropertyType DWORD -Force
```

**Verify:**
```powershell
Get-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem" `
  -Name "LongPathsEnabled"
```
→ `LongPathsEnabled` must be `1`.

### A2 — Set PowerShell execution policy

```powershell
Set-ExecutionPolicy RemoteSigned -Scope CurrentUser
```

**Verify:**
```powershell
Get-ExecutionPolicy
```
→ `RemoteSigned`

### A3 — Identify USB-serial chip

Look at the small chip near the USB port on your ESP32 DevKit V1:

| Chip | Driver |
|------|--------|
| **CP2102** (Silicon Labs) | [CP210x Universal Windows Driver](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) |
| **CH340** (WCH) | [CH341SER.EXE](https://www.wch.cn/download/CH341SER_EXE.html) |
| **CH9102** (WCH, newer) | Same driver as CH340 |

Install the matching driver.

### A4 — Connect ESP32 and verify COM port

1. Plug the ESP32 into a USB port via a **data-capable** cable (not charge-only).
2. Open **Device Manager** → *Ports (COM & LPT)*.
3. Look for `Silicon Labs CP210x USB to UART Bridge (COMx)` or `USB-SERIAL CH340 (COMx)`.

**Verify (PowerShell):**
```powershell
[System.IO.Ports.SerialPort]::getportnames()
```
→ Returns the COM port, e.g. `COM3`, `COM5`.

> No COM port? Try: different USB cable, different port, reinstall driver.

---

## Phase B — Install ESP-IDF

We install ESP-IDF **v5.3.2** — the last v5.x LTS release. It is stable,
well-documented, and fully supports ESP32.

### B1 — Create install directory

```powershell
mkdir C:\esp
```

### B2 — Clone ESP-IDF

```powershell
cd C:\esp
git clone --branch v5.3.2 --depth 1 --shallow-submodules `
  https://github.com/espressif/esp-idf.git
```

**Verify:**
```powershell
Test-Path C:\esp\esp-idf
```
→ `True`

### B3 — Run the installer

```powershell
cd C:\esp\esp-idf
.\install.ps1
```

This downloads the Xtensa toolchain (gcc, binutils, etc.) and creates a
Python virtual environment with all required packages.

**Duration:** 5–30 minutes depending on internet speed.

**Common failures:**
- **Antivirus blocking downloads** — temporarily exclude `C:\esp\`.
- **Python pip timeout** — retry, or set `PIP_INDEX_URL` to a local mirror.
- **Windows Defender SmartScreen** — click *Run anyway* or unblock the file.

**Verify:** The script exits without errors. The last output line says `Done!`.

---

## Phase C — Verify Toolchain

### C1 — Set environment variables for this session

Every new terminal must run this before using `idf.py`:

```powershell
cd C:\esp\esp-idf
.\export.ps1
```

This sets `IDF_PATH`, adds `idf.py` and the toolchain to `PATH`.

### C2 — Check ESP-IDF version

```powershell
idf.py --version
```
→ `v5.3.2`

### C3 — Check C compiler

```powershell
xtensa-esp32-elf-gcc --version
```
→ Shows gcc version (e.g. `xtensa-esp32-elf-gcc (crosstool-NG ...)`).

> If this fails, the toolchain download in Phase B3 may have failed.
> Re-run `.\install.ps1`.

---

## Phase D — Build & Flash (First Project)

### D1 — Create a test project

```powershell
cd C:\esp
idf.py create-project hello_csi
cd hello_csi
```

### D2 — Set target chip

```powershell
idf.py set-target esp32
```

### D3 — Build

```powershell
idf.py build
```

**Verify:** Exit code 0. The last lines show:
```
Project build complete. To flash, run:
idf.py -p PORT flash
```

### D4 — Flash to ESP32

Identify your COM port first (A4), then:

```powershell
idf.py -p COM5 flash
```
(Replace `COM5` with your port.)

**Verify:** Output ends with `Leaving... Hard resetting...`

> **If flashing fails:** Hold the **BOOT** button on the ESP32, press
> the **EN** button, release BOOT, then retry.

### D5 — Monitor serial output

```powershell
idf.py -p COM5 monitor
```

**Verify:** The ESP32 boots and prints:
```
Hello hello_csi! Start counter: 0
...
This is esp32 chip with 2 CPU core(s), WiFi/BT/BLE
...
```

Press `Ctrl+]` to exit the monitor.

### D6 — Save monitor output as evidence

```powershell
cd C:\esp\hello_csi
idf.py -p COM5 monitor > output.txt
```
Wait 10 seconds, press `Ctrl+]` to exit.

This file proves the toolchain works end-to-end.

---

## Phase E — Persistence (Optional)

To avoid typing `.\export.ps1` in every new terminal, add a PowerShell alias:

```powershell
if (!(Test-Path $PROFILE)) { New-Item -Path $PROFILE -Force }
Add-Content $PROFILE "Set-Alias idf42 C:\esp\esp-idf\export.ps1"
```

Then in any new terminal:
```powershell
idf42
```
activates the IDF environment.

---

## Verification Checklist

Copy this into an issue or comment to track progress:

```
Phase A
[ ] A1 — Long paths enabled
[ ] A2 — Execution policy set
[ ] A3 — Serial driver installed
[ ] A4 — ESP32 visible as COM port

Phase B
[ ] B1 — C:\esp directory created
[ ] B2 — ESP-IDF v5.3.2 cloned
[ ] B3 — install.ps1 completed without errors

Phase C
[ ] C1 — export.ps1 runs without errors
[ ] C2 — idf.py --version shows v5.3.2
[ ] C3 — xtensa-esp32-elf-gcc --version works

Phase D
[ ] D1 — Test project created
[ ] D2 — Target set to esp32
[ ] D3 — Build succeeds
[ ] D4 — Flash succeeds (ESP32 connected)
[ ] D5 — Monitor shows Hello World output
[ ] D6 — Output saved as evidence

M-1 COMPLETE
```
