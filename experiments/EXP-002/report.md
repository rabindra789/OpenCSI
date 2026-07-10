# EXP-002: CSI Signal Stability Characterization

## Experiment Metadata

| Field | Value |
|-------|-------|
| **Experiment ID** | EXP-002 |
| **Title** | CSI Signal Stability Characterization |
| **Status** | Analyzed |
| **Report Version** | 1.0 |
| **Hardware** | ESP32 DevKit V1 (ESP32-D0WD-V3 rev3.0) |
| **ESP-IDF Version** | v5.3.2 |
| **Firmware** | `firmware/m3_exp_002/` (framework v0.1.1) |
| **Date** | 2026-07-10 |
| **Author** | Rabindra Kumar Meher |
| **Environment** | Indoor, residential, single room |

## Research Question

How stable and repeatable is the ESP32 CSI signal under controlled (idle) conditions?

## Goal

Characterize the baseline behavior of the ESP32 CSI subsystem in a static environment with no intentional stimulus. Establish a reference for all future sensing experiments.

## Experimental Setup

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
| Laptop | Windows 11, connected to same AP |

### Firmware

Based on the v0.1.1 framework (`firmware/m3_exp_002/`). CSI configuration unchanged:

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

### Data Collected Per Callback

- Full `wifi_csi_info_t` field dump
- Full `wifi_pkt_rx_ctrl_t` field dump
- Hex dump of first 128 bytes of CSI payload
- CSV line: `CSV,<seq_num>,<timestamp_us>,<rx_seq>,<rssi_dbm>,<rate>,<sig_mode>,<mcs>,<cwb>,<ant>,<noise_floor_dbm>,<channel>,<wifi_timestamp_us>,<payload_len>`

### Firmware Additions (v0.1.1)

This experiment was the first to use the v0.1.1 framework additions:

- `transport_begin()` — prints experiment metadata and CSV header at capture start
- `transport_write_csi_record()` — updated with per-record `seq_num` and `timestamp_us`
- `transport_end()` — prints run completion marker
- Callback interval tracking: min, max, average, and max gap between callbacks

## Procedure

1. Place ESP32 in a static location (~3m line-of-sight to AP).
2. Ensure no intentional movement or network activity during capture.
3. Build firmware with scenario label.
4. Flash to ESP32.
5. Start serial capture.
6. Wait for experiment auto-complete (300s).
7. Stop capture, save raw output.
8. Extract CSV data and final stats.

### Captures Executed

| Capture | Label | Duration | Notes |
|---------|-------|----------|-------|
| 1 | idle-5min | 300s | Daytime baseline |
| 2 | idle-5min-repeat | 300s | Repeatability check |

A third capture (idle-5min-night) is planned for low-ambient-activity conditions.

## Observations

### Per-Capture Summary

| Metric | Capture 1 | Capture 2 |
|--------|-----------|-----------|
| Total CSI callbacks | 72 | 64 (62 + 2 post-completion) |
| Active duration | 282.0 s | 250.4 s |
| Avg callback rate | 0.255 /s | 0.256 /s |
| RSSI range | -82 to -75 dBm | -84 to -79 dBm |
| Interval min | 80.96 ms | 80.99 ms |
| Interval avg | 3972.5 ms | 3974.6 ms |
| Interval max | 27639.3 ms | 19558.5 ms |
| Max gap | 27.64 s | 19.56 s |
| Non-HT (sig_mode=0) | 63 | 55 |
| HT (sig_mode=1) | 9 | 9 |
| Antenna 0 | 72 | 64 |
| Channel | 6 | 6 |
| Noise floor | -97 dBm | -96 dBm |
| First callback at | 23.08 s | 60.31 s |
| Payload length | 128 bytes | 128 bytes |

### Key Observations

1. **Callback rate is consistent between runs** — 0.255 and 0.256 per second. The rate appears to be driven by AP beacon timing and ambient WiFi activity, not by the ESP32's internal timing.

2. **HT packet count is identical** — Exactly 9 HT packets in both 5-minute captures. This suggests HT CSI generation is triggered by a specific periodic event (possibly AP beacon bursts or repeating network traffic patterns).

3. **Interval floor is ~81 ms** — Both captures show a minimum interval of approximately 81 ms. This matches the minimum callback spacing observed in EXP-001 (paired callbacks ~1 ms apart within a packet, but ~81 ms between unrelated packets). This may be a hardware or driver limit.

4. **Average interval is ~4 seconds** — Both captures show ~3973 ms average. This aligns with beacon interval expectations (typical AP sends beacons every 102.4 ms, but only a subset generate CSI callbacks).

5. **First callback latency varies significantly** — 23.08 seconds in capture 1 vs 60.31 seconds in capture 2. The first CSI callback depends on when the AP sends a beacon or the ESP32 receives a WiFi frame, which depends on AP timing and ambient traffic.

6. **Max gap varies significantly** — 27.64 vs 19.56 seconds. These gaps likely correspond to periods with no suitable WiFi frames for CSI generation.

7. **Channel changed from EXP-001** — EXP-001 recorded channel 1. Both EXP-002 captures show channel 6. The AP likely changed channels between experiments (dynamic channel selection or router reboot).

8. **All callbacks on antenna 0** — Consistent with EXP-001. This board (single PCB trace antenna) never reported antenna 1.

9. **RSSI range stable within each run** — ±7 dBm (capture 1) and ±5 dBm (capture 2) range. Slightly wider than EXP-001's ±3 dBm within individual scenarios, likely due to the longer capture duration.

### Comparison with EXP-001 Idle Scenario

| Metric | EXP-001 (idle, 60s) | EXP-002 (capture 1, 300s) |
|--------|--------------------|--------------------------|
| Total CSI | 13 | 72 |
| Avg rate | 0.20 /s | 0.26 /s |
| RSSI range | -85 to -79 dBm | -82 to -75 dBm |
| non-HT / HT | 13 / 0 | 63 / 9 |
| Channel | 1 | 6 |
| Noise floor | -97 dBm | -97 dBm |

The callback rate is slightly higher in EXP-002 (0.26 vs 0.20 /s). This may be due to channel change (channel 6 vs 1) or different ambient WiFi activity.

## Results

1. **CSI callback interval is driven by external WiFi activity, not by the ESP32.** Under idle conditions, callbacks arrive at ~4 second average intervals matching AP beacon timing patterns. The rate is consistent across runs (0.255 vs 0.256 /s).

2. **HT packets appear at a fixed rate.** Nine HT CSI packets appeared in both 5-minute captures. This suggests a deterministic trigger for HT CSI generation rather than random ambient traffic.

3. **The minimum achievable callback interval is ~81 ms.** Below this interval, callbacks do not occur under our test conditions. This is consistent with a hardware or driver processing limit.

4. **RSSI variance increases with capture duration.** Over 5 minutes, RSSI varies by ±5–7 dBm compared to ±3 dBm over 60 seconds. This reflects environmental variation (temperature, AP power control, interference) rather than measurement noise.

5. **Channel allocation is not stable across weeks.** The AP changed from channel 1 (EXP-001, July 6) to channel 6 (EXP-002, July 10). Long-term experiments should verify the channel before each capture.

## Discussion

**Callback timing and AP beacons:** The ~4 second average interval strongly suggests that CSI callbacks are primarily driven by AP beacon frames. Standard WiFi beacons at 102.4 ms intervals would produce ~586 beacons in 60 seconds. EXP-001 observed 13 callbacks from ~586 beacons (approximately 2.2% of beacons generate a callback). The ESP32's CSI subsystem applies additional filtering beyond MAC address matching.

**HT packet source:** The consistent count of 9 HT packets in both captures suggests a specific periodic traffic source. Possibilities include: AP performing HT channel measurement, a nearby device using HT rates, or ESP32 self-generated management frames. This was not investigated further.

**First callback latency:** The large variance (23s vs 60s) suggests that CSI capture may not begin immediately after `esp_wifi_set_csi(true)` is called. The delay may depend on WiFi driver state, connection quality, or frame reception timing. This has implications for experiments that need to synchronize CSI collection with external events.

**Channel change:** Between EXP-001 (July 6) and EXP-002 (July 10), the AP changed from channel 1 to channel 6. This was noticed during data analysis, not during the experiment. Future experiments should log the active channel at capture start.

## Conclusions

1. Under idle conditions, CSI callbacks arrive at an average rate of approximately 0.25 per second, driven by ambient WiFi activity (primarily AP beacons). The rate is repeatable across runs.

2. The minimum callback interval under our test conditions is approximately 81 ms. This is consistent across both captures and may represent a hardware or driver timing constraint.

3. HT CSI packets appear at a consistent rate (9 per 5 minutes) under identical conditions, suggesting a deterministic source rather than random ambient traffic.

4. First callback latency varies significantly (23–60 seconds), which must be accounted for in experimental timing.

5. RSSI range expands with longer capture durations (±5–7 dBm over 5 minutes vs ±3 dBm over 60 seconds). This sets a baseline for motion detection thresholds.

6. The channel number is not stable across days. Future experiments should verify and log the channel before each capture.

## Limitations

1. **Two captures only.** A third (nighttime) capture was planned but not executed. Two data points provide limited confidence for statistical analysis.

2. **Single AP environment.** Results depend on this specific router's beacon timing, channel selection, and traffic patterns. Different APs may produce different results.

3. **Single hardware instance.** One ESP32 DevKit V1 board. Results may differ with other ESP32 variants.

4. **Channel 6 only.** Both captures happened to be on channel 6. We did not test on channel 1 or 11.

5. **No promiscuous mode.** The ESP32 was in STA mode. Monitor mode may capture more frames.

6. **No MAC frame classification.** Callbacks were classified by sig_mode and rate only. Frame type (beacon, probe, data, etc.) was not decoded.

7. **Latency between transport_begin and first callback.** The "first callback at" time includes WiFi connection time, not just CSI enable latency. These were not separated.

8. **Post-completion callbacks.** Two callbacks arrived after the stats task declared the experiment complete. This happens because `transport_write_csi_record` runs in the WiFi task context, not the stats task. The CSV data includes these callbacks but the firmware stats counter did not.

## Future Work

- Run a nighttime capture with minimal ambient WiFi activity
- Decode 802.11 frame types to identify which frames generate CSI
- Test with promiscuous/monitor mode enabled
- Measure CSI callback latency relative to `esp_wifi_set_csi(true)`
- Disable `dump_ack_en` to isolate ACK contribution from data frame contribution
- Repeat on channels 1, 6, 11
- Verify HT packet source by analyzing frame timing and content
- Run with explicit traffic stimulus to test callback rate scaling (original EXP-002 objective)

## Raw Evidence

Raw serial captures are preserved unmodified:

| File | Capture | Lines | Size |
|------|---------|-------|------|
| `data/exp-002-idle-5min-raw.txt` | Capture 1 (idle-5min) | — | ~86 KB |
| `data/exp-002-idle-5min-repeat-raw.txt` | Capture 2 (idle-5min-repeat) | — | ~78 KB |
| `data/exp-002-idle-5min.csv` | CSV data, capture 1 | 72 | ~2 KB |
| `data/exp-002-idle-5min-repeat.csv` | CSV data, capture 2 | 64 | ~2 KB |

### CSV Columns

From `transport_begin()`:

```
# columns: seq_num, timestamp_us, rx_seq, rssi_dbm, rate, sig_mode, mcs, cwb, ant, noise_floor_dbm, channel, wifi_timestamp_us, payload_len
```

## Firmware

The experiment firmware is at `firmware/m3_exp_002/main/main.c`. Based on framework v0.1.1 with experiment-specific callback interval tracking.
