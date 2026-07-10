# Roadmap

This document describes the planned evolution of OpenCSI.

These are goals, not promises. No dates are committed. Priorities may shift based on what the experiments reveal.

For detailed capability definitions, experiment-to-capability mapping, and design principles, see [docs/SENSING-ROADMAP.md](docs/SENSING-ROADMAP.md).

---

## v0.1.0 — Foundation ✅

Completed milestones:

- Development environment and toolchain verification
- Hardware workflow (build, flash, monitor)
- Raw CSI observation firmware
- Reusable firmware framework (WiFi, CSI, Transport)
- First published experiment (EXP-001)
- Project documentation and architecture

---

## v0.2.0 — Hardware Characterization

Planned:

- Additional CSI characterization experiments
- Better understanding of ESP32 CSI behavior
- Expanded experiment registry

---

## v0.3.0 — Signal Characterization

Planned:

- Static environment baseline
- CSI stability over hours
- Noise floor measurement

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
