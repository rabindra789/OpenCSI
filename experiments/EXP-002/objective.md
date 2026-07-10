# EXP-002: CSI Signal Stability Characterization

## Research Question

How stable and repeatable is the ESP32 CSI signal under controlled (idle) conditions?

## Goal

Characterize the baseline behavior of the ESP32 CSI subsystem in a static environment with no intentional stimulus. This establishes a reference for all future sensing experiments.

## Hypothesis

We expect the CSI callback stream to show variation even in a static environment. The sources of this variation include AP beacon timing, ambient WiFi activity from nearby networks, and hardware-level timing jitter. We do not yet know:

- How regularly CSI callbacks arrive
- How much RSSI varies in a static environment
- Which packet types (sig_mode, rate) appear at idle
- Whether the CSI buffer contents are consistent across callbacks
- How these metrics change over longer capture durations

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

Based on the v0.1.1 framework (`firmware/m3_exp_002/`):

- `transport_begin()` with experiment metadata and CSV header
- `transport_write_csi_record()` with per-record timestamp and sequence number
- Stats accumulator: total count, callback intervals, RSSI min/max, sig_mode histogram, rate histogram, antenna histogram
- Periodic interim stats (every 30s) and auto-final summary at 300s
- `transport_end()` on completion

CSI configuration (unchanged):

| Parameter | Value |
|-----------|-------|
| `lltf_en` | true |
| `htltf_en` | true |
| `stbc_htltf2_en` | true |
| `ltf_merge_en` | true |
| `channel_filter_en` | true |
| `manu_scale` | true |
| `shift` | 0 |
| `dump_ack_en` | true |

Source: `firmware/m3_exp_002/main/main.c`

## Procedure

1. Place ESP32 in a static location (~3m line-of-sight to AP).
2. Ensure no intentional movement or network activity during capture.
3. Build firmware with scenario label.
4. Flash to ESP32.
5. Start serial monitor, wait for WiFi connection.
6. Let experiment run for full duration.
7. After auto-complete, stop monitor, save raw output.
8. Extract CSV data and final stats.

### Captures

| Capture | Label | Duration | Purpose |
|---------|-------|----------|---------|
| 1 | idle-5min | 300s | Extended baseline |
| 2 | idle-5min-repeat | 300s | Repeatability check |
| 3 | idle-5min-night | 300s | Low-ambient-activity baseline |

### Measurements

| Metric | How measured |
|--------|-------------|
| Callback count | Running total |
| Callback interval | Difference between consecutive `timestamp_us` values |
| Average callback rate | `total_count / elapsed_s` |
| RSSI stability | Min, max, range over duration |
| RSSI time series | Per-callback RSSI values in CSV |
| Packet type distribution | sig_mode and rate histograms |
| Antenna distribution | ant0 vs ant1 count |
| First callback latency | Time from start to first CSI callback |
| Callback gap analysis | Maximum interval between consecutive callbacks |

## Data Files

- `data/exp-002-idle-5min-raw.txt`: Raw serial output, capture 1
- `data/exp-002-idle-5min-repeat-raw.txt`: Raw serial output, capture 2
- `data/exp-002-idle-5min-night-raw.txt`: Raw serial output, capture 3
- `data/exp-002-data.csv`: Consolidated per-packet CSV
- `objective.md`: This file
- `report.md`: Results, discussion, and conclusion (written after execution)

## Expected Output

The CSV header (from `transport_begin()`) will be:

```
# columns: seq_num, timestamp_us, rx_seq, rssi_dbm, rate, sig_mode, mcs, cwb, ant, noise_floor_dbm, channel, wifi_timestamp_us, payload_len
```

Each CSV line follows this format.
