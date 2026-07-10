# EXP-003: CSI Startup Timeline Decomposition

## Research Question

What causes the delay before the first CSI callback?

## Goal

Decompose the startup timeline into distinct stages. Identify which stage contributes the most to the total delay from power-on to the first CSI callback.

## Hypothesis

We do not have a hypothesis. The goal is to instrument each stage independently and let the evidence identify the dominant contributor.

Possible contributors (not assumptions):

- NVS initialization time
- WiFi hardware initialization
- WiFi association and handshake
- DHCP negotiation
- CSI hardware enable latency
- Wait for first receivable frame after CSI enable

## Design Principle

This experiment instruments multiple stages in the startup sequence.

This is not a violation of the "one question per experiment" principle.

The one question is: "What causes the first-callback delay?"

Multiple measurements are needed to answer that single question, just as EXP-002 needed multiple measurements (interval min, max, avg, RSSI range) to characterize stability.

If new questions arise (e.g., "does the delay change with AP distance?"), they will be recorded as future work rather than expanding this experiment.

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
| ESP32 IP | 192.168.29.140 (DHCP) |

### Firmware

Based on the framework v0.1.1 components (WiFi, CSI, Transport). No framework modifications.

The experiment adds a separate event handler to capture WiFi/IP event timestamps without modifying the framework.

Instrumentation:

| Stage | Timestamp label | Source |
|-------|----------------|--------|
| Entry to `app_main` | `t_app_main` | `esp_timer_get_time()` at function start |
| `wifi_init()` completed | `t_wifi_init_done` | `esp_timer_get_time()` after `wifi_init()` returns |
| WiFi STA started | `t_sta_start` | `WIFI_EVENT_STA_START` event |
| WiFi disconnected (if occurs) | `t_sta_disconnected` | `WIFI_EVENT_STA_DISCONNECTED` event |
| IP obtained | `t_got_ip` | `IP_EVENT_STA_GOT_IP` event |
| CSI enabled | `t_csi_enabled` | `esp_timer_get_time()` after `csi_init()` returns |
| First CSI callback | `t_first_csi` | `esp_timer_get_time()` in CSI callback |

Derived durations:

| Duration | Calculation | Meaning |
|----------|------------|---------|
| `t_wifi_init_done - t_app_main` | | Time to initialize WiFi stack |
| `t_sta_start - t_app_main` | | Time until WiFi hardware starts |
| `t_got_ip - t_sta_start` | | Association + DHCP time |
| `t_csi_enabled - t_got_ip` | | Time to enable CSI after connection |
| `t_first_csi - t_csi_enabled` | | Time from CSI enable to first callback |
| `t_first_csi - t_app_main` | | Total startup latency (matches EXP-002) |

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

### Data Collected

- Timestamped startup log with each stage labeled
- Decomposed timeline printed once at first CSI callback
- Same CSV output as v0.1.1 (seq_num, timestamp_us, per-callback fields)
- End-of-run summary with final stats and timeline

## Procedure

1. Flash firmware to ESP32.
2. Start serial capture.
3. Reset ESP32 (via hardware reset or power cycle).
4. Wait for first CSI callback, decomposed timeline is printed.
5. Let experiment run for 60 seconds after first callback (to verify CSI is working).
6. Stop capture, save raw output.
7. Repeat 5 times to measure variance.

### Captures

| Capture | Label | Purpose |
|---------|-------|---------|
| 1–5 | | Replicate to measure variance. No variables change between runs. |

Each capture is a separate flash + reset cycle.

## Measurements

| Metric | How measured |
|--------|-------------|
| Total startup latency | `t_first_csi - t_app_main` |
| Per-stage durations | Derived from timestamp differences |
| Dominant stage | The stage with the largest contribution to total delay |
| Variance across replicates | Range and standard deviation of each stage duration |

## Data Files

- `data/exp-003-runN-raw.txt`: Raw serial output for each run (N = 1..5)
- `data/exp-003-timeline.csv`: Consolidated per-run timing data
- `objective.md`: This file
- `report.md`: Results, discussion, and conclusion (written after execution)

## Expected Output

On first CSI callback, the firmware prints:

```
=== STARTUP TIMELINE ===
t_app_main:         0 us
t_wifi_init_done:   <N> us (+<delta> us)
t_sta_start:        <N> us (+<delta> us)
t_got_ip:           <N> us (+<delta> us)
t_csi_enabled:      <N> us (+<delta> us)
t_first_csi:        <N> us (+<delta> us)
========================
```

Each line shows the absolute timestamp and the delta from the previous stage.
