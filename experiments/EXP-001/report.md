# EXP-001: CSI Callback Source Characterization

## Experiment Metadata

| Field | Value |
|-------|-------|
| **Experiment ID** | EXP-001 |
| **Title** | CSI Callback Source Characterization |
| **Status** | Published |
| **Hardware** | ESP32 DevKit V1 (ESP32-D0WD-V3 rev3.0) |
| **ESP-IDF Version** | v5.3.2 |
| **Firmware Commit** | `6a92f66` (M1 baseline) + `firmware/m2_exp_001/` |
| **Date** | 2026-07-06 |
| **Author** | OpenCSI |
| **Environment** | Indoor, residential, single room |

## Research Question

What WiFi traffic actually generates CSI callbacks on the ESP32?

Current observations from M1 showed approximately 1-3 CSI callbacks per second, all non-HT frames. It was not known which network traffic was responsible for these callbacks or whether additional traffic would increase the rate.

## Goal

Identify the relationship between network activity and CSI callback generation by testing multiple traffic scenarios independently.

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
| Band / Channel | 2.4 GHz, channel 1, BW20 |
| AP | Integrated router, BSSID `8a:da:0c:b9:08:43` |
| ESP32 IP | 192.168.29.140 |
| Gateway | 192.168.29.1 |
| Laptop | Windows 11, connected to same AP |
| Distance | ~3m line-of-sight to AP |

### Firmware

Based on M1 (`firmware/m1_csi_observe/main/main.c`) with the following additions:

- Per-packet CSV summary line appended to each callback
- Stats accumulator tracking: total count, RSSI min/max, sig_mode histogram, rate histogram, antenna histogram
- Periodic interim stats print every 10 seconds
- Auto-generated final summary after 60 seconds
- Scenario label configurable at build time via `CONFIG_EXP_SCENARIO_LABEL`

CSI configuration (unchanged from M1):

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

Source: `firmware/m2_exp_001/main/main.c`

### Data Collected Per Callback

- Full `wifi_csi_info_t` field dump (MAC addresses, len, rx_seq, payload_len)
- Full `wifi_pkt_rx_ctrl_t` field dump (rssi, rate, sig_mode, mcs, cwb, ant, noise_floor, channel, timestamp, etc.)
- Hex dump of first 128 bytes (or up to `data->len`) of CSI payload
- CSV summary line: `CSV,<count>,<rx_seq>,<rssi>,<rate>,<sig_mode>,<mcs>,<cwb>,<ant>,<noise_floor>,<channel>,<timestamp>,<len>`

## Procedure

Each scenario was executed independently with a dedicated firmware build and flash cycle:

1. Update `CONFIG_EXP_SCENARIO_LABEL` in sdkconfig
2. Run `idf.py reconfigure && idf.py build`
3. Flash firmware: `idf.py -p COM6 flash`
4. Start serial capture: `esp_idf_monitor --disable-auto-color -p COM6 -b 115200`
5. Wait for ESP32 to connect to WiFi (printed "Connected. IP: 192.168.29.140")
6. Begin network traffic stimulus (if applicable)
7. Firmware auto-prints final summary after 60 seconds
8. Stop traffic, stop monitor, save raw output

### Scenarios

| # | Scenario | Stimulus | Rationale |
|---|----------|----------|-----------|
| 1 | idle | None | Establish baseline callback rate with ambient network activity only |
| 2 | ping-router | `ping 192.168.29.1 -t` from laptop | Test whether traffic between laptop and router triggers CSI |
| 3 | ping-esp32 | `ping 192.168.29.140 -t` from laptop | Test whether traffic addressed to ESP32 triggers CSI |
| 4 | download | `Invoke-WebRequest http://speedtest.tele2.net/10MB.zip` | Test whether high-bandwidth network activity triggers CSI |
| 5 | video | `ping 8.8.8.8 -n 60` from laptop | Test whether internet-bound traffic triggers CSI |

## Observations

### Per-Scenario Data

#### 1. Idle (no intentional traffic)

- 13 CSI callbacks in 60 seconds (first at 31.6s, last at 60.2s)
- All non-HT (sig_mode=0), rate 11, antenna 0
- RSSI range: -85 to -79 dBm
- Noise floor: -97 dBm, channel 1
- Callbacks arrived in pairs with identical rx_seq values

#### 2. Ping to Router (192.168.29.1)

- 6 CSI callbacks in 60 seconds
- All non-HT (sig_mode=0), rate 11, antenna 0
- RSSI range: -84 to -78 dBm
- Ping was confirmed successful (1-3ms RTT)

#### 3. Ping to ESP32 (192.168.29.140)

- 41 CSI callbacks in 60 seconds
- All non-HT (sig_mode=0)
- Rate distribution: 33 at rate 11, 8 at rate 10
- All antenna 0
- RSSI range: -85 to -78 dBm
- Multiple callbacks with identical rx_seq values and ~1ms spacing
- Callbacks arrived continuously during ping, not only at beacon intervals

#### 4. Large File Download

- 11 CSI callbacks in 60 seconds
- 9 non-HT (sig_mode=0), 2 HT (sig_mode=1)
- HT packets had len=384 bytes vs 128 bytes for non-HT
- All rate 11, antenna 0
- HT packets occurred at low rx_seq values (3-17)
- RSSI range: -86 to -80 dBm

#### 5. Video / Internet Traffic

- 12 CSI callbacks in 60 seconds
- 10 non-HT (sig_mode=0), 2 HT (sig_mode=1)
- Same HT pattern as download scenario
- All rate 11, antenna 0
- RSSI range: -87 to -81 dBm

### Summary Table

| Scenario | Duration | Total CSI | Avg/s | RSSI (dBm) | non-HT | HT | Rate 10 | Rate 11 | Ant0 | Ant1 |
|----------|----------|-----------|-------|------------|--------|----|---------|---------|------|------|
| idle | 60s | 13 | 0.20 | -85..-79 | 13 | 0 | 0 | 13 | 13 | 0 |
| ping-router | 60s | 6 | 0.09 | -84..-78 | 6 | 0 | 0 | 6 | 6 | 0 |
| ping-esp32 | 60s | 41 | 0.63 | -85..-78 | 41 | 0 | 8 | 33 | 41 | 0 |
| download | 60s | 11 | 0.17 | -86..-80 | 9 | 2 | 0 | 11 | 11 | 0 |
| video | 60s | 12 | 0.18 | -87..-81 | 10 | 2 | 0 | 12 | 12 | 0 |

### Observed Patterns

1. **Ping to ESP32 produced more CSI callbacks** than idle (41 vs 13). Ping to router (6) and download (11) produced similar or fewer callbacks than idle.

2. **Multiple callbacks with identical rx_seq** appeared during the ping-esp32 scenario. The time between these paired callbacks was approximately 1 ms (measured from PHY timestamps).

3. **HT CSI packets** (sig_mode=1, len=384 bytes) appeared during the download and video scenarios but did not appear during idle or ping scenarios.

4. **All callbacks across all scenarios** reported antenna 0.

5. **RSSI remained within ±3 dBm** within each scenario and ranged from -90 to -78 dBm across all scenarios.

6. **Noise floor was -96 to -97 dBm** across all measurements, channel 1.

7. **The ping-router scenario produced the lowest count** at 6 callbacks, lower than idle at 13. Both values are small enough that natural variance in AP beacon timing could account for the difference.

## Results

The following quantitative results were obtained from the five scenarios:

1. **Traffic type and callback count**: Scenarios with traffic not addressed to the ESP32 (idle, ping-router, download, video) produced 6-13 callbacks per 60 seconds (0.09-0.20/s). The scenario with traffic addressed to the ESP32 (ping-esp32) produced 41 callbacks (0.63/s), a 3-6x increase.

2. **Callback-to-packet ratio**: During the ping-esp32 scenario, Windows `ping -t` sends one ICMP echo request per second. Over ~28 seconds of active CSI time, approximately 28 pings were sent. The firmware recorded 41 callbacks, yielding approximately 1.5 callbacks per ping.

3. **HT packet occurrence**: HT CSI packets were only observed during the download and video scenarios. Both scenarios involved the laptop transmitting data over the WiFi network. Neither idle nor ping scenarios produced HT packets.

4. **Antenna**: All 83 callbacks across all scenarios reported antenna 0.

## Discussion

### Relationship between traffic and CSI callbacks

Under the tested configuration, CSI callbacks increased when packets were addressed to the ESP32. Traffic between other devices on the same network did not produce a comparable increase.

**Hypothesis**: The ESP32 WiFi driver filters received packets by destination MAC address before invoking the CSI callback. Only unicast packets addressed to the ESP32's station MAC, broadcast frames, and multicast frames (if subscribed) generate CSI callbacks. Packets between other stations are received at the PHY layer but filtered at the MAC layer before reaching the CSI callback.

This hypothesis is consistent with the observed data but has not been directly verified. Possible approaches for verification include enabling WiFi promiscuous mode and observing whether the callback rate changes, or examining the ESP-IDF WiFi driver source code.

### Callback pairs with identical rx_seq

Paired callbacks with identical rx_seq and ~1ms spacing were observed during the ping-esp32 scenario.

**Hypothesis**: With `dump_ack_en=true`, the ESP32 generates CSI for both the received packet and the acknowledgment (ACK) frame transmitted in response. The ACK is a distinct frame with its own PHY reception, so it generates a separate CSI callback. Both share the same rx_seq because the ACK acknowledges that specific sequence number.

Alternative explanations include the same packet being processed twice by the CSI subsystem, or a retransmission being counted as a separate callback. These alternatives have not been ruled out.

### HT packets during download and video

HT CSI packets were observed during the download and video scenarios but not during idle or ping. The mechanism responsible is currently unknown.

**Observations only**:
- HT packets had sig_mode=1 (HT), mcs=0 or 1, and cwb=0, 1, or 2
- HT CSI payload length was 384 bytes vs 128 bytes for non-HT
- HT packets occurred at low rx_seq values (3-17)
- HT packets appeared alongside non-HT packets with similar timestamps

**No explanation is proposed at this time.** The conditions for HT CSI generation may relate to HT Information Elements in AP beacons, association/reassociation frames, or HT-format data packets. Further experiments with controlled HT parameters are needed.

### The ping-router result

The ping-router scenario produced fewer callbacks (6) than idle (13). This may reflect natural variance in callback timing rather than a systematic difference. Both values come from a single 60-second observation. AP beacons are typically transmitted at 100 TU intervals (102.4 ms), yielding approximately 586 beacons per 60 seconds. The fact that only 6-13 of these generated CSI callbacks suggests that the ESP32 applies additional filtering beyond destination MAC address.

**Hypothesis**: The ESP32 may only generate CSI for a subset of received packets based on packet type (e.g., only QoS data frames, or only frames with specific frame control fields). This could explain why only a small fraction of the ~586 expected beacons generated callbacks.

## Conclusions

1. **Under these test conditions, CSI callbacks increased when the ESP32 received packets addressed to it**. Ping to ESP32 produced 3-6x more callbacks than idle. Traffic between other network devices did not increase the callback rate.

2. **The background callback rate (~0.2/s in this environment) reflects ambient WiFi activity**. It is not a firmware-imposed floor or ceiling. The rate changed when traffic directed at the ESP32 was introduced.

3. **HT CSI data is available on this hardware** (384 bytes per packet vs 128 for non-HT), but the conditions that trigger HT CSI generation are not yet understood.

4. **Antenna diversity was not observed** — all callbacks reported antenna 0 on a board with a single PCB trace antenna.

## Limitations

This experiment has the following limitations:

1. **Single hardware instance**: One ESP32 DevKit V1 board. Results may differ with other ESP32 variants (ESP32-S2, ESP32-C3, ESP32-S3) or board designs with external antennas.

2. **Single WiFi environment**: One residential location, one AP/router, one channel (1). Results may differ in other environments (office, industrial, outdoor) or with different AP hardware/firmware.

3. **Single observation per scenario**: Each scenario was run once for 60 seconds. No repeat measurements were taken. The observed callback counts have unknown variance.

4. **No promiscuous mode testing**: The ESP32 was configured as a WiFi station (STA mode). Promiscuous mode or monitor mode may produce different callback behavior.

5. **No broadcast/multicast isolation**: The experiment did not separately test broadcast or multicast traffic. The extent to which these frame types contribute to the observed callback count is unknown.

6. **No MAC-level frame classification**: The experiment did not decode 802.11 frame types (beacon, probe, data, null, etc.). Callbacks were classified only by sig_mode, rate, RSSI, and antenna.

7. **Limited traffic stimulus**: Network traffic was generated from a single laptop. Results may differ with multiple devices, different traffic patterns (VoIP, streaming, IoT), or different ping implementations.

8. **CSI callback filtering mechanism unknown**: The specific conditions under which the ESP32 firmware invokes the CSI callback are not documented in the publicly available ESP-IDF sources. The conclusions above are based on observed behavior, not confirmed through code analysis.

9. **All measurements at 115200 baud serial**: Serial bandwidth limits the rate at which full callback data can be transmitted. Internal firmware counters are unaffected, but detailed per-packet CSV data may be lost if callback rate exceeds serial throughput.

## Future Work

The following research directions are possible based on this experiment:

- **Callback rate scaling**: Vary the incoming packet rate to the ESP32 and measure the CSI callback response curve. Determine whether the relationship is linear and identify any saturation point.

- **HT packet investigation**: Design a controlled experiment to determine what specifically triggers HT CSI generation. Possible variables: HT capabilities in AP beacons, HT-format data frames, channel width (20 vs 40 MHz).

- **ACK vs data frame isolation**: Disable `dump_ack_en` and compare callback rates to isolate the contribution of ACK frames.

- **MAC filter characterization**: Test broadcast, multicast, and promiscuous mode to understand the ESP32's CSI callback filtering logic.

- **Multiple channel testing**: Repeat the experiment on different WiFi channels (1, 6, 11) and in non-overlapping channel conditions.

- **Multiple environment testing**: Repeat the experiment in different physical environments (office, outdoor, multi-AP) to characterize environmental effects.

- **Statistical replication**: Repeat the idle and ping scenarios multiple times to measure variance and establish confidence intervals.

## Raw Evidence

Raw serial captures are preserved unmodified in the `data/` directory:

| File | Scenario | Lines | Size |
|------|----------|-------|------|
| `data/idle/exp-001-idle-raw.txt` | Idle | 701 | ~45 KB |
| `data/ping-router/exp-001-ping-router-raw.txt` | Ping to router | 476 | ~31 KB |
| `data/ping-esp32/exp-001-ping-esp32-raw.txt` | Ping to ESP32 | 1947 | ~127 KB |
| `data/download/exp-001-download-raw.txt` | Large file download | 6311 | ~412 KB |
| `data/video/exp-001-video-raw.txt` | Video / internet traffic | 868 | ~56 KB |
| `data/exp-001-data.csv` | Consolidated per-packet data | 96 | ~4 KB |

### Data Format

Each raw file contains the full serial output including:
- Bootloader messages
- WiFi connection logs
- Per-callback full field dumps
- CSV summary lines (prefixed with `CSV,`)
- Interim and final stats sections

The consolidated CSV file contains one row per CSI callback from all scenarios with the columns: `scenario, count, rx_seq, rssi_dbm, rate, sig_mode, mcs, cwb, ant, noise_floor_dbm, channel, timestamp_us, len_bytes`.

## Firmware

The experiment firmware is at `firmware/m2_exp_001/main/main.c`. It is a minimal instrumentation layer over the M1 baseline (`firmware/m1_csi_observe/main/main.c`). Key components:

- CSI callback with full field dump (preserved from M1)
- CSV line appended to each callback
- Stats accumulator in global `csi_stats_t` struct
- `stats_task`: periodic interim stats (10s interval) and auto-final summary at 60s
- `blink_task`: LED heartbeat on GPIO2 (preserved from M1)
- Scenario label and duration configurable via Kconfig (`CONFIG_EXP_SCENARIO_LABEL`, `CONFIG_EXP_DURATION_SECONDS`)
