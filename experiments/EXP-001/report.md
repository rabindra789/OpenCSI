# EXP-001: CSI Callback Source Characterization

## Experiment Metadata

| Field | Value |
|-------|-------|
| **Experiment ID** | EXP-001 |
| **Title** | CSI Callback Source Characterization |
| **Status** | Peer Reviewed (Internal) |
| **Report Version** | 1.0 |
| **Hardware** | ESP32 DevKit V1 (ESP32-D0WD-V3 rev3.0) |
| **ESP-IDF Version** | v5.3.2 |
| **Firmware Commit** | `6a92f66` (M1 baseline) + `firmware/m2_exp_001/` |
| **Date** | 2026-07-06 |
| **Author** | Rabindra Kumar Meher |
| **Environment** | Indoor, residential, single room |

## Research Question

What WiFi traffic actually generates CSI callbacks on the ESP32?

Current observations from M1 showed approximately 1-3 CSI callbacks per second, all non-HT frames. We did not know which network traffic caused these callbacks or whether more traffic would increase the rate.

## Goal

Understand how different types of network traffic affect CSI callback generation by testing multiple scenarios.

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

We executed each scenario independently with a dedicated firmware build and flash cycle:

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

1. **Traffic type and callback count**: Scenarios with traffic not addressed to the ESP32 (idle, ping-router, download, video) produced 6-13 callbacks per 60 seconds (0.09-0.20/s). The scenario with traffic addressed to the ESP32 (ping-esp32) produced 41 callbacks (0.63/s), a 3-6x increase.

2. **Callback-to-packet ratio**: During the ping-esp32 scenario, Windows `ping -t` sends one ICMP echo request per second. Over ~28 seconds of active CSI time, approximately 28 pings were sent. The firmware recorded 41 callbacks, yielding approximately 1.5 callbacks per ping.

3. **HT packet occurrence**: HT CSI packets were only observed during the download and video scenarios. Both scenarios involved the laptop transmitting data over the WiFi network. Neither idle nor ping scenarios produced HT packets.

4. **Antenna**: All 83 callbacks across all scenarios reported antenna 0.

## Discussion

**Traffic and callbacks**: CSI callbacks increased only for packets addressed to the ESP32. One possible explanation — the WiFi driver filters by destination MAC before invoking the callback. Traffic between other devices is received at PHY but filtered at MAC. We did not verify this directly.

**Callback pairs (identical rx_seq)**: With `dump_ack_en=true`, the ESP32 likely generates CSI for both the received packet and its ACK. The ACK is a distinct PHY reception sharing the same rx_seq (it acknowledges that sequence number). Alternatives (double processing, retransmission) not ruled out.

**HT packets**: Observed during download/video but not idle/ping. Mechanism unknown. Payload 384 bytes vs 128 bytes non-HT. No hypothesis proposed.

**Ping-router (6 vs 13 idle)**: Both single observations; natural variance in AP beacon timing may account for the difference. Only 6-13 of ~586 expected beacons generated callbacks, suggesting the ESP32 applies additional filtering beyond destination MAC.

## Conclusions

1. Under our test conditions, CSI callbacks increased when the ESP32 received packets addressed to it. Ping to ESP32 produced 3-6x more callbacks than idle. Traffic between other devices did not increase the callback rate.

2. The background callback rate (~0.2/s in this environment) reflects ambient WiFi activity. We did not find evidence of a firmware-imposed floor or ceiling. The rate changed when we sent traffic to the ESP32.

3. HT CSI data is available on this hardware (384 bytes per packet vs 128 for non-HT). We have not yet identified what triggers HT CSI generation.

4. We did not observe antenna diversity. All callbacks reported antenna 0 on this board with a single PCB trace antenna.

## Limitations

Our experiment has these limitations:

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

### Experiments

- **Callback rate scaling**: Vary incoming packet rate to find saturation point.
- **HT packet investigation**: Determine triggers for HT CSI generation.
- **ACK vs data frame isolation**: Disable `dump_ack_en` to isolate ACK contribution.
- **MAC filter characterization**: Test broadcast, multicast, promiscuous mode.
- **Multi-channel / multi-environment testing**: Repeat on channels 1, 6, 11 and in office, outdoor, multi-AP settings.
- **Statistical replication**: Repeat idle and ping to establish confidence intervals.

### Engineering

- **Structured logging**: Replace ad-hoc CSV with machine-parseable output format.
- **Higher UART throughput**: Increase baud rate to accommodate denser callback rates.

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

The experiment firmware is at `firmware/m2_exp_001/main/main.c`. We built it as a minimal instrumentation layer over the M1 baseline (`firmware/m1_csi_observe/main/main.c`). Key components:

- CSI callback with full field dump (preserved from M1)
- CSV line appended to each callback
- Stats accumulator in global `csi_stats_t` struct
- `stats_task`: periodic interim stats (10s interval) and auto-final summary at 60s
- `blink_task`: LED heartbeat on GPIO2 (preserved from M1)
- Scenario label and duration configurable via Kconfig (`CONFIG_EXP_SCENARIO_LABEL`, `CONFIG_EXP_DURATION_SECONDS`)
