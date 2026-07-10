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

## v0.2.0 — Hardware Characterization ✅

Completed:

- CSI callback source characterization (EXP-001)
- CSI signal stability baseline (EXP-002)
- CSI startup timeline decomposition (EXP-003)
- Understanding of ESP32 CSI behavior and limitations
- Experiment registry with reproducible procedures

---

## v0.3.0 — Signal Characterization 🟢

Active:

- EXP-004: Traffic-driven CSI sampling characterization
- EXP-005: Static environmental variation
- EXP-006: Human movement baseline

Goals:

- Understand how CSI changes when we deliberately change one variable at a time
- Build the evidence foundation for future sensing algorithms
- No detection, no classification, no ML — pure characterization

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
