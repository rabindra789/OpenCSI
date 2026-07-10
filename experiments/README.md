# OpenCSI Research Timeline

## Overview

This directory tracks all OpenCSI research. Each experiment answers one specific question about the ESP32 CSI subsystem through reproducible measurements.

## Lifecycle

| Stage | Description |
|-------|-------------|
| **Proposed** | Research question identified, experiment design drafted |
| **Planned** | Objective and procedure reviewed, ready to execute |
| **Running** | Data collection in progress |
| **Analyzed** | Raw data processed, observations documented |
| **Reviewed** | Results reviewed, discussion validated |
| **Peer Reviewed (Internal)** | Full report finalized with limitations and future work |
| **Archived** | Experiment closed, raw evidence preserved |

## Principles

- Every experiment answers exactly one research question
- Firmware changes are minimal — only the instrumentation needed to answer the question
- No infrastructure, protocols, parsers, storage, or optimization is built during an experiment
- Observations are separated from hypotheses and conclusions
- Results are reproducible: hardware, network, firmware, and procedure are fully documented
- Raw evidence is preserved unmodified after collection

---

## Research Timeline

| Date | ID | Title | Status |
|------|----|-------|--------|
| 2026-07-06 | [EXP-001](EXP-001/report.md) | CSI Callback Source Characterization | Peer Reviewed (Internal) |
| 2026-07-10 | [EXP-002](EXP-002/report.md) | CSI Signal Stability Characterization | Reviewed (Internal) |
| 2026-07-10 | [EXP-003](EXP-003/report.md) | CSI Startup Timeline Decomposition | Reviewed (Internal) |
| 2026-07-10 | [EXP-004](EXP-004/report.md) | Traffic-Driven CSI Sampling Characterization | Analyzed |
| — | [EXP-005](EXP-005/objective.md) | Static Environmental Variation | Proposed |
| — | [EXP-006](EXP-006/objective.md) | Human Movement Baseline | Proposed |

---

## Directory Structure

```
experiments/
├── README.md              This file — central research index
├── EXP-001/               CSI Callback Source Characterization (Peer Reviewed)
│   ├── report.md
│   └── data/
├── EXP-002/               CSI Signal Stability Characterization (Reviewed)
│   ├── report.md
│   └── data/
├── EXP-003/               CSI Startup Timeline Decomposition (Reviewed)
│   ├── report.md
│   └── data/
├── EXP-004/               Traffic-Driven CSI Sampling Characterization (Proposed)
│   ├── objective.md
│   └── data/
├── EXP-005/               Static Environmental Variation (Proposed)
│   ├── objective.md
│   └── data/
└── EXP-006/               Human Movement Baseline (Proposed)
    ├── objective.md
    └── data/
```

## How to Read an Experiment Report

Each report contains:

1. **Metadata** — author, version, hardware, firmware, date, environment
2. **Research Question** — what the experiment investigates
3. **Setup** — hardware, network, and firmware configuration
4. **Procedure** — step-by-step execution
5. **Observations** — raw data presented without interpretation
6. **Results** — processed data and summary tables
7. **Discussion** — what the results suggest (hypotheses vs conclusions)
8. **Conclusions** — what is definitively supported by the evidence
9. **Limitations** — what the experiment does *not* claim
10. **Future Work** — possible research directions and engineering improvements
