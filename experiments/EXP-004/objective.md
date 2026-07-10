# EXP-004: Traffic-Driven CSI Sampling Characterization

## Research Question

How does controlled network traffic influence CSI sampling behaviour?

## Goal

Determine whether callback timing, callback rate, HT/non-HT distribution, and RSSI behave predictably when the ESP32 receives controlled traffic. Only the traffic type and rate change. All other variables (hardware, location, AP, duration) remain constant.

## Motivation

EXP-002 established the idle baseline (~0.25 callbacks/s driven by ambient WiFi). EXP-001 showed that ping to ESP32 produces more callbacks than idle. Neither experiment controlled the input traffic rate or measured the interval distribution.

Before we can interpret CSI changes caused by human movement or environment, we need to understand how the sampling process itself changes under different traffic loads. A motion detection algorithm that assumes a constant callback rate will fail if the rate changes with network activity.

## Hypothesis

We expect that:

1. Callback rate scales with incoming packet rate up to a saturation point.
2. Callback intervals become more regular under sustained traffic.
3. HT packet ratio increases under TCP/UDP traffic (which uses HT rates).
4. RSSI remains stable regardless of traffic rate (antenna-level characteristic).
5. The minimum callback interval of ~81 ms (observed in EXP-002) persists even under heavy traffic.

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
| Band | 2.4 GHz |
| AP | Integrated router |
| ESP32 IP | 192.168.29.140 |

### Firmware

Based on the framework v0.1.1 components (WiFi, CSI, Transport). No framework modifications.

The experiment adds:

- **Interval histogram:** 10 configurable time bins for callback interval distribution
- **Per-callback CSV** via standard transport (seq_num, timestamp_us, all fields)
- **Stats accumulator:** count, RSSI range, sig_mode histogram, rate histogram, antenna histogram

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

### Traffic Generation

| Traffic type | Method | Tool | Effective rate |
|-------------|--------|------|---------------|
| Idle | No intentional traffic | — | 0 packets/s |
| 1 ping/s | `ping -n <count> 192.168.29.140` (1s interval) | Windows ping | ~1 pkt/s |
| 10 UDP/s | UDP datagrams at 10 Hz to closed port | `tools/udp_sender.py` | ~10 pkt/s |
| 100 UDP/s | UDP datagrams at 100 Hz to closed port | `tools/udp_sender.py` | ~100 pkt/s |
| Continuous ping | `ping -t 192.168.29.140` (unbounded) | Windows ping | ~1 pkt/s (sustained) |
| TCP stream | TCP data at 10 Hz to port 8888 | `tools/tcp_stimulus.py` | Sustained throughput |
| Idle (repeat) | No intentional traffic | — | 0 packets/s |

## Procedure

For each traffic scenario:

1. Build firmware with scenario label.
2. Flash to ESP32.
3. Start serial capture.
4. Wait for WiFi connection (monitor output).
5. Start traffic stimulus from laptop.
6. Run for fixed duration (60 seconds after first CSI callback).
7. Stop traffic, stop capture, save raw output.
8. Extract CSV data and final stats.

### Scenarios

| # | Label | Traffic | Rate | Duration | Rationale |
|---|-------|---------|------|----------|-----------|
| 1 | idle | None | 0 pkt/s | 60s | Baseline (replicate EXP-002 short) |
| 2 | ping-1s | 1 ping/s | ~1 pkt/s | 60s | Low-rate stimulus |
| 3 | udp-10 | 10 UDP/s | ~10 pkt/s | 60s | Medium-rate stimulus |
| 4 | udp-100 | 100 UDP/s | ~100 pkt/s | 60s | High-rate stimulus |
| 5 | ping-continuous | Continuous ping | ~1 pkt/s (sustained) | 60s | Sustained low-rate |
| 6 | tcp-download | TCP stream | Sustained throughput | 60s | TCP protocol traffic |
| 7 | idle-repeat | None | 0 pkt/s | 60s | Post-stimulus baseline |

### Measurements

| Metric | How measured |
|--------|-------------|
| Total callbacks | Running count |
| Callback rate | `total_count / elapsed_s` |
| Interval histogram | 10-bin distribution (see below) |
| Interval min/avg/max/stddev | From timestamp_us differences |
| RSSI min/max/range | Per-callback RSSI tracking |
| Sig_mode distribution | non-HT (0) vs HT (1) count |
| Rate distribution | Per-rate histogram |
| Antenna distribution | ant0 vs ant1 count |
| First callback latency | Time from start to first callback |

### Histogram Bins

The interval histogram bins are defined in the firmware as follows:

| Bin | Range | Rationale |
|-----|-------|-----------|
| 1 | < 50 ms | Below observed floor — should not occur |
| 2 | 50–100 ms | Covers ~81 ms floor from EXP-002 |
| 3 | 100–500 ms | Short intervals under traffic |
| 4 | 500 ms – 1 s | Medium intervals |
| 5 | 1–2 s | Moderate intervals |
| 6 | 2–5 s | Typical ~4 s idle intervals |
| 7 | 5–10 s | Longer gaps |
| 8 | 10–30 s | ~20–28 s gaps seen in EXP-002 |
| 9 | 30–60 s | Extended gaps |
| 10 | > 60 s | Expedition gaps — should be rare |

### CSV Format

Each callback produces one CSV line with these columns:

`CSV,seq_num,timestamp_us,rx_seq,rssi_dbm,rate,sig_mode,mcs,cwb,ant,noise_floor_dbm,channel,wifi_timestamp_us,payload_len`

## Data Files

- `data/exp-004-<scenario>-raw.txt`: Raw serial output for each scenario
- `data/exp-004-data.csv`: Consolidated per-packet CSV across all scenarios
- `objective.md`: This file
- `report.md`: Results, discussion, and conclusion

## Expected Impact

This experiment will determine:

1. Whether future experiments need to account for traffic-dependent callback rates.
2. Whether sustained traffic produces more regular (lower-variance) callback intervals, which would improve signal processing.
3. Whether HT packet occurrence is predictable under controlled traffic.
4. The maximum sustainable CSI callback rate before serial or CPU saturation.

These results directly inform the design of future experiments and, eventually, sensing algorithms.
