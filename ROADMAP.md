# Roadmap

This document describes the planned evolution of OpenCSI.

These are goals, not promises. No dates are committed. Priorities may shift based on what the experiments reveal.

For detailed capability definitions, experiment-to-capability mapping, and design principles, see [docs/SENSING-ROADMAP.md](docs/SENSING-ROADMAP.md).

---

## v0.1.x — Foundation ✅

Completed milestones:

- Development environment and toolchain verification
- Hardware workflow (build, flash, monitor)
- Raw CSI observation firmware
- Reusable firmware framework (WiFi, CSI, Transport)
- First published experiment (EXP-001)
- Project documentation and architecture

---

## v0.2.0 — Signal Acquisition & Characterization ✅

Four experiments characterizing the ESP32 CSI acquisition pipeline, network traffic effects, and environmental sensitivity.

### Completed

- **EXP-002** — Signal stability: idle callback rate ~0.25/s, min interval ~81ms, first-callback latency 23-60s
- **EXP-003** — Startup timeline: WiFi association dominates startup time (4-56s), CSI enable ~8ms
- **EXP-004** — Traffic effects: callback rate saturates at ~10/s (serial bottleneck), HT ratio traffic-driven
- **EXP-005** — Environmental effects: RSSI and callback rate respond to physical changes (metal object: -5 dBm, -73% rate)

### Framework

- WiFi, CSI, Transport components: stable, frozen at v0.1.1
- No framework modifications across all experiments

---

## v0.3.0 — Human Signal Characterization 🟡

Planned:

- EXP-006: Human movement baseline — characterize how CSI changes when a person moves through the environment
- No classification, no detection, no ML — pure characterization
- Build the evidence required to design the first motion detection algorithm

---

## v0.4.0 — Motion Characterization

Planned:

- Walking vs. empty characterization
- Motion sensitivity limits
- Environment impact on motion signatures

---

## v0.5.0 — Presence Detection

Possible:

- First reference implementation
- Algorithm evaluation

---

## v0.6.0 — Activity Characterization

Planned:

- Walking vs. sitting vs. standing
- Activity transitions
- Multi-device sensing exploration

---

## v1.0.0

A stable, documented, open-source WiFi CSI human sensing framework for ESP32.
