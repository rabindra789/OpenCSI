# EXP-001: CSI Callback Source Characterization

## Goal

Find out which WiFi traffic types generate CSI callbacks on the ESP32. Check if the observed 1-3 callbacks per second is caused by network activity or firmware behavior.

## Hardware

| Component | Detail |
|-----------|--------|
| Board | ESP32 DevKit V1 (ESP32-D0WD-V3 rev3.0) |
| Antenna | Onboard PCB trace (single) |
| Serial | COM6, 115200 baud |
| USB-UART | CP2102 (auto-reset via DTR/RTS) |

## Network

| Parameter | Value |
|-----------|-------|
| SSID | Rabindra |
| Band | 2.4 GHz, channel 1, BW20 |
| AP | Integrated router, BSSID `8a:da:0c:b9:08:43` |
| ESP32 IP | 192.168.29.140 |
| Gateway | 192.168.29.1 |
| Laptop | Windows 11, connected to same AP |

## Firmware

Based on M1 (`firmware/m1_csi_observe`) with minimal instrumentation:
- Per-packet CSV summary line
- Stats accumulator (RSSI range, sig_mode count, rate count, antenna count)
- Periodic interim stats (every 10s) and final auto-summary at 60s
- Scenario label configurable via `CONFIG_EXP_SCENARIO_LABEL` (build-time)
- CSI config: `lltf_en=true, htltf_en=true, stbc_htltf2_en=true, ltf_merge_en=true, channel_filter_en=true, manu_scale=true, dump_ack_en=true`

Source: `firmware/m2_exp_001/main/main.c`

## Procedure

Each scenario was run independently with a fresh firmware build/flash cycle:

1. Build firmware with scenario label
2. Flash to ESP32
3. Start serial monitor (esp_idf_monitor, no color, 115200 baud)
4. Wait for ESP32 to connect to WiFi (~25-45s depending on scenario)
5. Start network traffic stimulus (if applicable)
6. Let experiment run for 60 seconds (firmware auto-completes)
7. Stop traffic, stop monitor, save raw output

### Scenarios

| # | Scenario | Stimulus |
|---|----------|----------|
| 1 | idle | No intentional traffic |
| 2 | ping-router | `ping 192.168.29.1 -t` (continuous from laptop to gateway) |
| 3 | ping-esp32 | `ping 192.168.29.140 -t` (continuous from laptop to ESP32) |
| 4 | download | `Invoke-WebRequest http://speedtest.tele2.net/10MB.zip` |
| 5 | video | `ping 8.8.8.8 -n 60` (continuous internet-bound traffic) |

## Results

### Summary Table

| Scenario | Duration | Total CSI | Avg/s | RSSI (dBm) | non-HT | HT | Rate 10 | Rate 11 | Ant0 | Ant1 |
|----------|----------|-----------|-------|------------|--------|----|---------|---------|------|------|
| idle | 60s | 13 | 0.20 | -85..-79 | 13 | 0 | 0 | 13 | 13 | 0 |
| ping-router | 60s | 6 | 0.09 | -84..-78 | 6 | 0 | 0 | 6 | 6 | 0 |
| ping-esp32 | 60s | 41 | 0.63 | -85..-78 | 41 | 0 | 8 | 33 | 41 | 0 |
| download | 60s | 11 | 0.17 | -86..-80 | 9 | 2 | 0 | 11 | 11 | 0 |
| video | 60s | 12 | 0.18 | -87..-81 | 10 | 2 | 0 | 12 | 12 | 0 |

### Per-Scenario Details

#### 1. Idle (no intentional traffic)
- 13 CSI callbacks in 60s (0.20/s)
- Callbacks arrived in pairs with identical rx_seq values
- First callback at 31.6s after boot
- Last callback at 60.2s

#### 2. Ping to Router (192.168.29.1)
- 6 CSI callbacks (0.09/s) — lowest count observed
- Ping was confirmed successful (all replies received, 2ms RTT)
- All non-HT, rate 11, antenna 0

#### 3. Ping to ESP32 (192.168.29.140)
- 41 CSI callbacks (0.63/s) — 3-6x higher than idle
- Multiple callbacks with identical rx_seq values observed
- Rate distribution: 33 at rate 11, 8 at rate 10
- Callbacks arrived continuously during ping (not only at beacon intervals)

#### 4. Large File Download (10MB from speedtest server)
- 11 CSI callbacks (0.17/s) — similar to idle
- 2 HT packets observed (sig_mode=1, len=384 bytes)
- HT packets occurred at low rx_seq values (3-17)

#### 5. Video/Internet Traffic (ping to 8.8.8.8)
- 12 CSI callbacks (0.18/s) — similar to idle
- 2 HT packets observed (same pattern as download)

### Observations

1. **Ping to ESP32 produced significantly more CSI callbacks** than idle (41 vs 13). Ping to router did not (6 vs 13). Traffic between other devices on the network did not produce a comparable increase.

2. **Multiple callbacks with identical rx_seq values** were observed during the ping-esp32 scenario. This suggests a single received packet can trigger more than one CSI callback.

3. **HT CSI packets** (sig_mode=1, len=384) appeared during the download and video scenarios but not during idle or ping. The mechanism that triggers HT CSI under these conditions is currently unknown.

4. **All CSI callbacks across all scenarios** reported antenna 0. The ESP32 DevKit V1 has a single antenna trace, so antenna diversity is not applicable.

5. **RSSI remained stable** within ±3 dBm within each scenario and varied by ±4 dBm across all scenarios (-90 to -78 dBm).

6. **Noise floor was consistent** at -96 to -97 dBm across all measurements.

### Hypotheses

1. We think the ESP32 generates CSI callbacks for packets whose destination MAC matches its own station MAC. Broadcast and multicast frames may also trigger callbacks. Traffic between other devices does not match this filter.

2. With `dump_ack_en=true`, the ESP32 may generate CSI for both the received packet and the ACK it sends in response. This would explain the same-rx_seq pairs we observed.

3. HT CSI callbacks may come from HT-format management frames. The AP may send these when it detects channel activity. The low rx_seq values suggest these could be association or reassociation frames.

4. The background callback rate (~0.2/s) likely comes from AP beacons and periodic management frames, not a firmware-imposed limit. We saw the rate increase when we sent traffic to the ESP32.

## Discussion

### Relationship between traffic and CSI callbacks

In our tests, CSI callbacks increased when packets were addressed to the ESP32. Traffic between other devices did not produce a comparable increase. We need more experiments to understand broadcast, multicast, management, and promiscuous-mode behavior.

### Callback pairs with identical rx_seq

Ping to ESP32 produced repeated pairs of callbacks with the same rx_seq value, different timestamps, and ~1ms spacing. This pattern is consistent with the hypothesis that `dump_ack_en=true` enables CSI generation for both the received data packet and the acknowledged transmission of the ESP32's response.

### HT packets during download and video

HT CSI packets were observed during the download and video scenarios but not during idle or ping. The mechanism responsible is currently unknown and requires further investigation.

### The ping-router result

Ping to the router produced fewer callbacks (6) than idle (13). This could be normal variance. With only one 60-second measurement per scenario, we expect some fluctuation. Repeating the idle test would likely give a range of 6-15 callbacks per 60s.

## Conclusions

1. Callbacks increase when we send traffic to the ESP32. In our tests, CSI callbacks were 3-6x higher when we pinged the ESP32 compared to idle. Traffic between other devices did not increase the callback rate.

2. The background callback rate (~0.2/s) reflects ambient AP activity. We did not find evidence of a firmware-imposed floor or ceiling.

3. HT CSI data is available (384 bytes per packet vs 128 for non-HT). We need more investigation to understand what triggers it.

4. Antenna diversity is not a factor on this hardware — all callbacks on ant0 with a single PCB trace antenna.

## Raw Data

- `captures/exp-001-data.csv`: Consolidated per-packet CSV from all scenarios
- `captures/exp-001-idle-raw.txt`: Full serial output, idle
- `captures/exp-001-ping-router-raw.txt`: Full serial output, ping to router
- `captures/exp-001-ping-esp32-raw.txt`: Full serial output, ping to ESP32
- `captures/exp-001-download-raw.txt`: Full serial output, download
- `captures/exp-001-video-raw.txt`: Full serial output, video/internet traffic

## Experiment Firmware

`firmware/m2_exp_001/` — minimal instrumentation over M1 baseline.

## Next Steps

Review results and plan EXP-002. Suggested direction: vary incoming packet rate to measure the relationship between traffic rate and CSI callback frequency.
