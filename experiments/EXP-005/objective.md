# EXP-005: Static Environmental Variation

## Research Question

How do controlled changes in the physical environment affect the CSI signal?

## Goal

Characterize how CSI metrics change when we deliberately change one static environmental variable at a time. The purpose is understanding, not detection.

Only one environmental variable changes per run. No movement. No traffic stimulus.

## Motivation

EXP-002 established that idle CSI signals have stable RSSI and a predictable callback rate. EXP-004 showed that callback rate saturates at ~10/s and HT ratio is traffic-dependent. But all previous characterization was done in one specific environment.

If we intend to build sensing algorithms that work across environments, we must first understand how the baseline signal changes under different environmental conditions. Without this, a future algorithm could interpret "door closed" as "person present."

## Layout

```
+--------------------------+
|                          |
|   Window (if present)    |
|                          |
|       +--------+         |
|       | Desk   |         |
|       | [ESP]  |         |
|       +--------+         |
|         |                |
|         | ~2m            |
|         |                |
|   +----------+           |
|   |   Door   |           |
|   +----------+           |
|                          |
+--------------------------+
```

- ESP32 on desk, USB-connected, antenna facing room center
- Router position: fixed (integrated router)
- Door: room entrance, ~2m from ESP32
- All measurements: no intentional traffic (idle)

## Scenarios

Each scenario is a single physical change from the baseline.

| # | Label | Environmental change | Rationale |
|---|-------|---------------------|-----------|
| 1 | empty-baseline | Room as-is, door open | Baseline |
| 2 | door-closed | Door closed | Attenuation through door |
| 3 | door-open | Door fully opened | Fresnel zone change |
| 4 | metal-object | Large metal object placed 1m from ESP32 | Strong reflector |
| 5 | chair-added | Metal-frame chair placed 1m from ESP32 | Medium reflector |

## Measurements

Same metrics as EXP-004, plus per-second RSSI variance from post-processing:

| Metric | Source |
|--------|--------|
| Total callbacks | FINAL STATS |
| Callback rate | FINAL STATS |
| RSSI min/max/range | FINAL STATS |
| RSSI per-second variance | CSV timestamp + RSSI |
| HT vs non-HT ratio | FINAL STATS |
| Interval histogram | FINAL STATS |
| Antenna distribution | FINAL STATS |

No per-packet CSI phase or amplitude analysis. Those are future experiments.

## Procedure

1. Set up environment to match baseline.
2. Flash with scenario label (idle traffic, 60s).
3. Capture for 180s (cover WiFi connection + 60s experiment).
4. No traffic stimulus.
5. Save raw output.
6. Change one environmental variable.
7. Repeat for each scenario.
8. Return to baseline. Run empty-baseline-repeat to verify stability.

## Firmware

Same `firmware/m5_exp_004` binary as EXP-004 idle scenario. No changes. Scenario label identifies the environmental condition.

## Data

- `data/exp-005-<scenario>-raw.txt` for each scenario
- `report.md` with per-scenario analysis

## Engineering Impact

This experiment will determine whether CSI metrics are sensitive enough to detect static environmental changes. If RSSI changes detectably with door position or object placement, simple threshold-based detection may be feasible. If not, we need CSI phase data for future sensing.
