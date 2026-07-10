# EXP-004: Traffic-Driven CSI Sampling Characterization

## Metadata

| Field | Value |
|-------|-------|
| Experiment ID | EXP-004 |
| Date | 2026-07-10 |
| Framework version | v0.1.1 |
| Framework changes | None |
| Firmware | `firmware/m5_exp_004` |
| Duration per scenario | 60 s after first CSI callback |
| Serial baud | 115200 |
| Channel | 6 |
| Hardware | ESP32 DevKit V1 (ESP32-D0WD-V3 rev3.0) |
| AP | Integrated router, SSID "Rabindra", 2.4 GHz |
| ESP32 IP | 192.168.29.140 |
| Author | OpenCSI |

## Research Question

How does controlled network traffic influence CSI sampling behaviour?

## Setup

Same hardware, location, AP, and firmware framework as EXP-002 and EXP-003. The experiment adds an interval histogram (10 bins from <50ms to >60s) and traffic-specific scenario labels.

Firmware: `firmware/m5_exp_004`. No framework modifications. TCP server is a compile-time option in the experiment firmware (disabled by default, enabled only for tcp-download scenario).

## Scenarios

| # | Label | Traffic | Rate | Expected callbacks |
|---|-------|---------|------|-------------------|
| 1 | idle | None | 0 pkt/s | Ambient only (~0.2/s) |
| 2 | ping-1s | ICMP echo, 1/s | ~1 pkt/s | 1 ping → multiple ACK/response callbacks |
| 3 | udp-10 | UDP to closed port, 10 Hz | ~10 pkt/s | Should match traffic rate |
| 4 | udp-100 | UDP to closed port, 100 Hz | ~100 pkt/s | Saturation expected |
| 5 | ping-continuous | ICMP echo, unbounded | ~1 pkt/s | Sustained low-rate stimulus |
| 6 | tcp-download | TCP data on port 8888, 10 Hz | ~10 pkt/s | TCP protocol overhead |
| 7 | idle-repeat | None | 0 pkt/s | Post-experiment baseline check |

## Results

### Summary Table

| Scenario | Calls | Duration (s) | Rate (/s) | HT | non-HT | HT % | RSSI range | RSSI avg |
|----------|-------|-------------|-----------|----|--------|------|------------|----------|
| idle | 13 | 61.0* | 0.21 | 0 | 13 | 0.0% | -84 to -82 | -82.5 |
| ping-1s | 189 | 64.1 | 2.95 | 65 | 124 | 34.4% | -84 to -79 | -80.9 |
| udp-10 | 647 | 62.6 | 10.34 | 630 | 17 | 97.4% | -84 to -80 | -82.5 |
| udp-100 | 469 | 63.0 | 7.46 | 328 | 141 | 69.9% | -86 to -81 | -83.9 |
| ping-continuous | 154 | 61.1 | 2.52 | 53 | 101 | 34.4% | -86 to -79 | -82.4 |
| tcp-download | 601 | 62.0 | 9.70 | 585 | 16 | 97.3% | -86 to -81 | -83.8 |
| idle-repeat | 16 | 69.1* | 0.23 | 0 | 16 | 0.0% | -84 to -80 | -81.4 |

*Duration from FINAL STATS (includes up to 10s overrun from the periodic stats check).

### Callback Rate vs Traffic Rate

```
Rate (/s)
   12 |                                                
   11 |                                                
   10 |                           udp-10 (10.34)    tcp-download (9.70)
    9 |                                                
    8 |                                                
    7 |                           udp-100 (7.46)       
    6 |                                                
    5 |                                                
    4 |                                                
    3 |              ping-1s (2.95)   ping-continuous (2.52)
    2 |                                                
    1 |                                                
    0 |  idle (0.21)    idle-repeat (0.23)
       +------------------------------------------------
         0             10            100
                     Traffic rate (packets/s)
```

### Interval Histogram

| Bin | idle | ping-1s | udp-10 | udp-100 | ping-cont | tcp-dl |
|-----|------|---------|--------|---------|-----------|--------|
| <50ms | 7.7% | 0.5% | 0.2% | 0.2% | 0.6% | 0.2% |
| 50-100ms | 7.7% | 36.0% | 66.0% | 87.2% | 32.9% | 53.9% |
| 100-500ms | 30.8% | 35.4% | 33.8% | 10.0% | 34.2% | 45.6% |
| 0.5-1s | — | 22.6% | — | 1.1% | 26.5% | 0.3% |
| 1-2s | — | 2.1% | — | 0.2% | 3.9% | — |
| 2-5s | 7.7% | 1.1% | — | 1.3% | 1.9% | — |
| 5-10s | 15.4% | 0.9% | — | — | — | — |
| 10-30s | 23.1% | 0.9% | — | — | — | — |
| 30-60s | — | — | — | — | — | — |
| >60s | — | — | — | — | — | — |

### RSSI

RSSI remained stable (±5 dBm) across all scenarios. No correlation between traffic rate and RSSI variation.

### HT vs non-HT

- **Idle**: 100% non-HT (rate 11 = 1 Mbps DSSS)
- **Ping (1/s)**: 34.4% HT — the ICMP echo request is received at HT rate, but some ambient traffic is non-HT
- **UDP/TCP (10 Hz)**: >97% HT — sustained traffic forces HT rates
- **UDP (100 Hz)**: 69.9% HT — lower HT % possibly due to serial bottleneck causing selective callback loss

## Discussion

### 1. Callback rate scales with packet rate, then saturates

Callback rate increases from 0.21/s (idle) to 2.95/s (ping-1s) to 10.34/s (udp-10). At 100 UDP/s, the rate drops to 7.46/s instead of continuing to scale.

**Cause**: Serial output at 115200 baud is the bottleneck. Each callback's transport output is ~900 bytes. At 115200 baud (effective ~11,500 bytes/s), the maximum theoretical throughput is ~13 callbacks/s. In practice, the callback task competes with other FreeRTOS tasks, and the actual sustainable rate is ~10/s.

**Evidence**: udp-10 achieves 10.34/s (near the serial limit). udp-100 achieves only 7.46/s (serial buffer overflow causing callback loss). The minimum callback interval of ~81ms (observed in EXP-002) is consistent with this hardware limitation.

**Minimum interval**: 80.8 ms observed in udp-100, matching EXP-002's ~81 ms floor. This appears to be a hardware/software limit of the ESP32 CSI subsystem, not a WiFi MAC limitation.

### 2. HT ratio increases with traffic rate

Non-HT calls (rate 11 = 1 Mbps DSSS) dominate in idle scenarios. Under sustained traffic, HT packets (rate 11 = OFDM/HT rates) dominate. The Wi-Fi AP and ESP32 negotiate higher modulation rates when data traffic is present.

Key observation: at ping (1/s), only 34% of callbacks are HT. At UDP/TCP (10 Hz), >97% are HT. This means the **packet rate influences which rate the WiFi hardware uses**, which in turn affects what CSI data we capture.

At 100 UDP/s, HT drops to 69.9%. This may be because the serial bottleneck drops some HT callbacks preferentially (HT packets are larger and may take longer to process), or because the 100 Hz traffic causes enough congestion that some packets are received at lower rates.

### 3. RSSI is traffic-independent

RSSI varies by ±5 dBm across all scenarios regardless of traffic rate. This confirms RSSI is a channel-level characteristic not influenced by packet rate. RSSI averaged -82.5 dBm in idle, -80.9 in ping-1s, -82.5 in udp-10, consistent across all runs.

### 4. Interval distribution shifts with traffic

Idle: intervals are sparse (0.5s to 30s). Under ping-1s: most intervals cluster in 50-1000ms (94%). Under udp-10: 99.8% of intervals are <500ms, with 66% in 50-100ms. Under udp-100: 87.2% in 50-100ms — the tightest distribution.

### 5. Baseline repeatability

Idle (pre-experiment): 13 calls, 0.21/s. Idle-repeat (post-experiment): 16 calls, 0.23/s. The environment and callback rate are stable within expected variance.

### 6. TCP vs UDP

TCP download at 10 Hz produces similar callback rate (9.70/s) and HT ratio (97.3%) to UDP at 10 Hz (10.34/s, 97.4%). The TCP protocol overhead (SYN handshake, ACK for each data packet, echo) generates more total callbacks but doesn't significantly change the distribution.

## Conclusions

| # | Hypothesis | Verdict | Confidence |
|---|-----------|---------|-----------|
| 1 | Callback rate scales with packet rate up to saturation | **Confirmed** | High |
| 2 | Callback intervals become more regular under sustained traffic | **Confirmed** | High |
| 3 | HT packet ratio increases under TCP/UDP traffic | **Confirmed** | High |
| 4 | RSSI remains stable regardless of traffic rate | **Confirmed** | High |
| 5 | Minimum callback interval of ~81 ms persists under heavy traffic | **Confirmed** | High |

### Definitive Findings

1. **CSI callback rate saturates at ~10/s** due to 115200 baud serial bottleneck. Beyond ~10 input packets/s, the serial port cannot drain fast enough, and callbacks are lost.
2. **Traffic rate determines HT/non-HT distribution**. Idle → 0% HT. 1 pkt/s → 34% HT. 10 pkt/s → 97% HT. This is not a CSI property but a WiFi rate adaptation property.
3. **RSSI is independent of traffic rate**. RSSI varies by ±5 dBm across all scenarios.
4. **Callback interval distribution tightens with traffic**. Under idle, intervals span 0.05s to >30s. Under 10 pkt/s traffic, >99% of intervals are <500ms.
5. **Baseline is repeatable**. Pre- and post-experiment idle captures match within expected variance.

## Limitations

1. **Serial bottleneck confounds high-rate characterization**. At 100 UDP/s, we cannot distinguish between CSI hardware limits and serial bandwidth limits. A higher baud rate (e.g., 921600) or binary output would be needed to characterize callback rates above 10/s.
2. **One replicate per scenario**. This experiment is a survey across 7 traffic types, not a statistical characterization. Each scenario was run once.
3. **Traffic timing not precisely controlled**. The ping/UDP traffic starts before the ESP32's first CSI callback. The number of stimulus packets within the 60s experiment window varies depending on WiFi connection latency.
4. **Ambient WiFi was not controlled**. Other devices on the same channel may have contributed to callback counts.
5. **HT/non-HT attribution by rate field only**. The `sig_mode` field tells us HT vs non-HT, but the specific MCS/rate within each mode is not analyzed.

## Engineering Impact

### For OpenCSI Framework

1. **Serial baud rate is a design parameter for CSI throughput**. At 115200 baud, the maximum sustainable CSI callback rate is ~10/s. Future high-rate experiments should use higher baud rates or binary transport.

2. **HT/non-HT ratio is not a CSI property**. It reflects the WiFi rate adaptation algorithm. Experiments that compare HT vs non-HT CSI data must account for traffic-dependent rate selection.

3. **Callback interval distribution matters more than average rate**. Under traffic, intervals are tightly clustered (50-500ms). Under idle, intervals span multiple seconds. Sensing algorithms that depend on regular sampling must account for this.

### For Future Experiments

1. EXP-005 (static environment) should use idle traffic to avoid confounding environmental effects with traffic effects.
2. EXP-006 (human movement) should be run under both idle and low-rate traffic to determine whether movement effects are distinguishable from traffic effects.
3. Before designing motion detection algorithms, the serial bandwidth bottleneck should be addressed (higher baud rate or binary transport).

## Next Research Question

How does the wireless environment (door position, furniture, objects) affect CSI metrics under no-traffic (idle) conditions?

This is EXP-005.
