# EXP-003: CSI Startup Timeline Decomposition

## Experiment Metadata

| Field | Value |
|-------|-------|
| **Experiment ID** | EXP-003 |
| **Title** | CSI Startup Timeline Decomposition |
| **Status** | Reviewed (Internal) |
| **Report Version** | 1.0 |
| **Hardware** | ESP32 DevKit V1 (ESP32-D0WD-V3 rev3.0) |
| **ESP-IDF Version** | v5.3.2 |
| **Firmware** | `firmware/m4_exp_003/` (framework v0.1.1, no framework modifications) |
| **Date** | 2026-07-10 |
| **Author** | Rabindra Kumar Meher |
| **Environment** | Indoor, residential, single room |

## Research Question

What causes the delay before the first CSI callback?

## Goal

Decompose the startup timeline into distinct stages and identify which stage contributes the most to the total delay from power-on to the first CSI callback.

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
| ESP32 IP | 192.168.29.140 (DHCP) |

### Firmware

Based on the framework v0.1.1 components (WiFi, CSI, Transport). No framework modifications were made.

The experiment instruments the startup sequence by recording timestamps at each stage:

| Stage | How measured |
|-------|-------------|
| `app_main` | `esp_timer_get_time()` at function entry |
| `wifi_init done` | `esp_timer_get_time()` after `wifi_init()` returns |
| `IP acquired` | `esp_timer_get_time()` inside `on_wifi_connected` callback |
| `CSI enabled` | `esp_timer_get_time()` after `csi_init()` returns |
| `first CSI` | `esp_timer_get_time()` in first CSI callback |

Derived durations:
- **wifi_init_call:** `wifi_init done - app_main`
- **connect+DHCP:** `IP acquired - wifi_init done`
- **csi_enable:** `CSI enabled - IP acquired`
- **wait_first_csi:** `first CSI - CSI enabled`

The framework's `wifi_event_handler` prints "Disconnected. Reconnecting..." on each `WIFI_EVENT_STA_DISCONNECTED` event. The number of disconnect events during initial association is used as a proxy for connection difficulty.

### Data Collected

- Timestamped startup log with each stage labeled
- Decomposed timeline printed at first CSI callback
- Connection retry count from framework log output
- CSV output for all callbacks after startup (not used in this analysis)

## Procedure

1. Flash firmware to ESP32.
2. Start serial capture.
3. Reset ESP32 via flash tool's hard reset (RTS pin).
4. Wait for first CSI callback — decomposed timeline is printed.
5. Run for 60 seconds after first callback.
6. Stop capture, save raw output.
7. Repeat 5 times.

No variables were changed between runs.

### Captures

| Run | File | Notes |
|-----|------|-------|
| 1 | `data/exp-003-run1-raw.txt` | |
| 2 | `data/exp-003-run2-raw.txt` | |
| 3 | `data/exp-003-run3-raw.txt` | |
| 4 | `data/exp-003-run4-raw.txt` | |
| 5 | `data/exp-003-run5-raw.txt` | Extreme reconnect case |

## Observations

### Per-Run Timeline (absolute times from app_main entry)

| Run | wifi_init done | IP acquired | CSI enabled | first CSI | Total |
|-----|---------------|-------------|-------------|-----------|-------|
| 1 | 0.268 s | 4.462 s | 4.470 s | 4.520 s | **4.520 s** |
| 2 | 0.398 s | 4.982 s | 4.990 s | 5.048 s | **5.048 s** |
| 3 | 0.267 s | 7.912 s | 7.920 s | 7.967 s | **7.967 s** |
| 4 | 0.400 s | 23.961 s | 23.970 s | 24.012 s | **24.063 s** |
| 5 | 0.267 s | 56.672 s | 56.680 s | 56.730 s | **56.730 s** |

### Per-Stage Duration Breakdown

| Run | wifi_init_call | connect+DHCP | csi_enable | wait_first_csi | Disconnects |
|-----|---------------|-------------|------------|----------------|-------------|
| 1 | 0.268 s (5.9%) | **4.194 s (92.8%)** | 0.008 s (0.2%) | 0.050 s (1.1%) | 2 |
| 2 | 0.398 s (7.9%) | **4.584 s (90.8%)** | 0.008 s (0.2%) | 0.058 s (1.1%) | 2 |
| 3 | 0.267 s (3.4%) | **7.645 s (96.0%)** | 0.008 s (0.1%) | 0.047 s (0.6%) | 4 |
| 4 | 0.400 s (1.7%) | **23.561 s (98.1%)** | 0.008 s (0.0%) | 0.042 s (0.2%) | 4 |
| 5 | 0.267 s (0.5%) | **56.405 s (99.4%)** | 0.008 s (0.0%) | 0.049 s (0.1%) | 30 |

### Stage Statistics Across 5 Runs

| Stage | Min | Max | Average | Std Dev |
|-------|-----|-----|---------|---------|
| wifi_init_call | 0.267 s | 0.400 s | 0.320 s | 0.069 s |
| connect+DHCP | 4.194 s | 56.405 s | 19.278 s | 22.827 s |
| csi_enable | 0.008 s | 0.008 s | 0.008 s | 0.000 s |
| wait_first_csi | 0.042 s | 0.058 s | 0.049 s | 0.006 s |
| **Total** | **4.520 s** | **56.730 s** | **19.666 s** | **22.844 s** |

### Key Observations

1. **`connect+DHCP` is the dominant stage in every run.** It accounts for 90–99% of total startup time. The other stages combined contribute at most 10%.

2. **`connect+DHCP` variance is extreme.** The fastest connection was 4.2 seconds, the slowest was 56.4 seconds. This 13× spread is driven by the number of disconnect/reconnect cycles during association.

3. **`wifi_init_call` is stable.** 0.27–0.40 seconds across all runs. This includes NVS initialization, event loop creation, and WiFi driver init.

4. **`csi_enable` is rock-solid.** 8,400–8,500 microseconds (8.4 ms) in every run. CSI hardware configuration and enable is deterministic.

5. **`wait_first_csi` is stable.** 42–58 ms after CSI is enabled. Once CSI capture is active, the first receivable WiFi frame arrives within ~50 ms.

6. **Disconnect count correlates with connect time.** Runs with 2 disconnects took ~4–5 seconds. Runs with 4 disconnects took ~8–24 seconds. Run 5 with 30 disconnects took ~56 seconds. Each disconnect/reconnect cycle adds approximately 1–6 seconds.

7. **The disconnect events occur during initial association, not after.** They are `WIFI_EVENT_STA_DISCONNECTED` events that fire before `WIFI_EVENT_STA_CONNECTED`. The framework's handler calls `esp_wifi_connect()` on each disconnect, creating a retry loop.

## Results

1. **The dominant contributor to first-callback delay is WiFi association + DHCP, accounting for 90–99% of total startup time.** All other stages combined contribute less than 10%.

2. **CSI enable latency is negligible** (8 ms) and deterministic. The CSI subsystem does not cause startup delay.

3. **The time from CSI enable to first callback is small and stable** (~50 ms). Once CSI is enabled and the ESP32 is connected to WiFi, a receivable frame arrives within milliseconds.

4. **The large variance in total startup time (4.5–56.7 seconds) is caused by WiFi association retries.** Each disconnect/reconnect cycle during initial association adds significant delay.

5. **The 23–60 second range observed in EXP-002's first-callback latency is explained by WiFi association variance.** The CSI hardware and callback chain are not the bottleneck.

## Conclusions

1. The delay before the first CSI callback is caused by the WiFi connection process (association + DHCP), not by CSI hardware initialization or frame availability. Specifically:

   - **CSI is not the bottleneck.** CSI enables in ~8 ms and receives its first callback within ~50 ms.
   - **WiFi association is the bottleneck.** It accounts for 90–99% of total startup time and varies dramatically between runs.

2. The first CSI callback cannot arrive until WiFi is fully connected. This is because `csi_init()` is called from `on_wifi_connected()`, which fires only after `IP_EVENT_STA_GOT_IP`.

3. The ESP32 experiences `WIFI_EVENT_STA_DISCONNECTED` events during initial association. Each disconnect triggers a reconnect attempt via the framework's handler. The number of retries varies between 2 and 30 in our test environment.

4. The root cause of these disconnect events during association was not investigated. Possible causes include AP congestion, WiFi interference, AP power-saving mode, or association handshake timeout.

5. From a sensing application perspective: if an experiment needs to synchronize CSI collection with a specific event, the startup delay is unpredictable and entirely driven by WiFi association timing. The CSI subsystem itself is fast and deterministic.

## Limitations

1. **Association retry mechanism not instrumented.** We counted disconnect events but did not decode the reason codes or measure the time between individual retries. The exact behavior of the disconnect/reconnect loop is handled by ESP-IDF's WiFi driver, which we did not modify or instrument.

2. **Single AP environment.** These results are specific to this router. A different AP may have different association timing, different retry behavior, or no retries at all.

3. **DHCP and association were not separated.** The `connect+DHCP` stage includes both WiFi association (802.11 handshake) and DHCP (IP address negotiation). We did not instrument these separately.

4. **Hardware reset via RTS pin only.** Each run was triggered by the flash tool's hard reset (RTS toggling). A power-cycle reset or software reset may produce different behavior.

5. **No AP configuration changes.** The AP was running its default configuration. We did not test with different security modes (WPA2 vs WPA3), different beacon intervals, or different DTIM periods.

6. **Static IP not tested.** Using a static IP instead of DHCP would eliminate the DHCP negotiation stage but may not affect the association retry behavior.

7. **Single hardware instance.** Results may differ with other ESP32 board variants or revisions.

## Future Work

- Decode disconnect reason codes to understand why association fails
- Separate DHCP from association timing (e.g., by using a static IP)
- Test with different APs or AP configurations
- Test with software reset vs. power-on reset
- Test with `esp_wifi_set_ps(WIFI_PS_NONE)` to disable power saving (may affect association timing)
- Determine whether disconnect events are caused by AP congestion, channel interference, or protocol timeout
- Design a CSI-enabled experiment that does not depend on WiFi connection for CSI initialization (e.g., use promiscuous/monitor mode)

## Raw Evidence

Raw serial captures are preserved unmodified:

| File | Run | Size |
|------|-----|------|
| `data/exp-003-run1-raw.txt` | 1 | |
| `data/exp-003-run2-raw.txt` | 2 | |
| `data/exp-003-run3-raw.txt` | 3 | |
| `data/exp-003-run4-raw.txt` | 4 | |
| `data/exp-003-run5-raw.txt` | 5 | |

## Firmware

The experiment firmware is at `firmware/m4_exp_003/main/main.c`. Based on the framework v0.1.1 components with no modifications. The experiment registers timestamp points in `app_main`, `on_wifi_connected`, and the CSI callback.
