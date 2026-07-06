# EXP-002: CSI Callback Rate Characterization

## Research Question

How does incoming packet rate affect CSI callback frequency?

## Goal

Check if CSI callback frequency scales linearly with incoming traffic or hits a hardware or driver limit.

## Hypothesis

CSI callback frequency will scale proportionally with incoming packet rate up to a saturation point set by the WiFi task processing capacity or the serial output bandwidth. Beyond this point, callbacks may be dropped, merged, or show unusual timing.

## Setup

### Hardware

| Component | Detail |
|-----------|--------|
| Board | ESP32 DevKit V1 (ESP32-D0WD-V3 rev3.0) |
| Antenna | Onboard PCB trace (single) |
| Serial | COM6, 115200 baud |
| USB-UART | CP2102 (auto-reset via DTR/RTS) |

### Network

| Parameter | Value |
|-----------|-------|
| SSID | Rabindra |
| Band | 2.4 GHz, channel 1, BW20 |
| AP | Integrated router, BSSID `8a:da:0c:b9:08:43` |
| ESP32 IP | 192.168.29.140 |
| Laptop | Windows 11, connected to same AP |

### Firmware

Same instrumentation firmware as EXP-001 (`firmware/m2_exp_001/`):
- Per-packet CSV summary line
- Stats accumulator (RSSI range, sig_mode count, rate count, antenna count)
- Periodic interim stats (every 10s) and final auto-summary at 60s
- CSI config unchanged: `lltf_en=true, htltf_en=true, stbc_htltf2_en=true, ltf_merge_en=true, channel_filter_en=true, manu_scale=true, dump_ack_en=true`

### Stimulus

A Python script (`experiments/EXP-002/data/udp_sender.py`) sends UDP packets to the ESP32 on port 9999 at controlled rates. The ESP32's LWIP stack receives these packets and the WiFi hardware generates CSI callbacks. The higher-layer ICMP Port Unreachable response may trigger additional ACK-based CSI callbacks via `dump_ack_en`.

## Procedure

For each rate level:

1. Build firmware with scenario label (`low-rate`, `med-rate`, `high-rate`)
2. Flash to ESP32
3. Start serial monitor, wait for WiFi connection
4. Start UDP sender at target rate for 60 seconds
5. Wait for experiment auto-complete (60s duration)
6. Stop sender, stop monitor, save raw output
7. Extract CSV data and final stats

### Rate Levels

| Level | Target Rate | Packets in 60s | Stimulus |
|-------|-------------|----------------|----------|
| Low | ~1 pkt/s | ~60 | UDP sender at 1 Hz |
| Medium | ~10 pkt/s | ~600 | UDP sender at 10 Hz |
| High | ~100 pkt/s | ~6000 | UDP sender at 100 Hz |

### Measurements

- Total CSI callbacks
- Average callbacks per second (over active CSI period)
- RSSI range
- sig_mode distribution
- Rate distribution
- Any dropped, delayed, or unexpected callback behavior

## Data Files

- `data/udp_sender.py`: Python script for controlled-rate UDP transmission
- `data/exp-002-low-rate-raw.txt`: Raw serial output, low rate
- `data/exp-002-med-rate-raw.txt`: Raw serial output, medium rate
- `data/exp-002-high-rate-raw.txt`: Raw serial output, high rate
- `data/exp-002-data.csv`: Consolidated per-packet CSV
- `objective.md`: This file (research question, goal, hypothesis, setup, procedure)
- `results.md`: Results, discussion, and conclusion (written after execution)
