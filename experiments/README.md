# OpenCSI Experiment Registry

## Overview

This directory is the central index of all OpenCSI research. Every experiment investigates one specific question about the ESP32 CSI subsystem through controlled, reproducible measurements.

## Lifecycle

| Stage | Description |
|-------|-------------|
| **Proposed** | Research question identified, experiment design drafted |
| **Planned** | Objective and procedure reviewed, ready to execute |
| **Running** | Data collection in progress |
| **Analyzed** | Raw data processed, observations documented |
| **Reviewed** | Results reviewed, discussion validated |
| **Published** | Full report finalized with limitations and future work |
| **Archived** | Experiment closed, raw evidence preserved |

## Principles

- Every experiment answers exactly one research question
- Firmware changes are minimal — only the instrumentation needed to answer the question
- No infrastructure, protocols, parsers, storage, or optimization is built during an experiment
- Observations are separated from hypotheses and conclusions
- Results are reproducible: hardware, network, firmware, and procedure are fully documented
- Raw evidence is preserved unmodified after collection

---

## Full Registry

| ID | Title | Status | Summary | Hardware | ESP-IDF | Report | Data | Firmware | Date |
|----|-------|--------|---------|----------|---------|--------|------|----------|------|
| EXP-001 | CSI Callback Source Characterization | **Published** | CSI callbacks increased when packets were addressed to ESP32; traffic between other devices did not produce a comparable increase | ESP32 DevKit V1 (ESP32-D0WD-V3 rev3.0) | v5.3.2 | [report](EXP-001/report.md) | [data](EXP-001/data/) | [m2_exp_001](../firmware/m2_exp_001/) | 2026-07-06 |
| EXP-002 | CSI Callback Rate Characterization | **Planned** | *pending* | — | — | — | — | — | — |
| EXP-003 | *(available)* | Proposed | — | — | — | — | — | — | — |

---

## Directory Structure

```
experiments/
├── README.md              This file — central research index
├── EXP-001/               Published experiment
│   ├── report.md          Full report with observations, analysis, limitations
│   └── data/              Raw evidence organized by scenario
├── EXP-002/               Planned experiment
│   ├── objective.md       Research question, hypothesis, procedure
│   └── data/              (populated during execution)
└── EXP-003/               (available for future work)
```

## How to Read a Published Experiment

Each published experiment contains:

1. **Metadata** — hardware, firmware, date, environment
2. **Research Question** — what the experiment investigates
3. **Setup** — hardware, network, and firmware configuration
4. **Procedure** — step-by-step execution
5. **Observations** — raw data presented without interpretation
6. **Results** — processed data and summary tables
7. **Discussion** — what the results suggest (hypotheses vs conclusions)
8. **Conclusions** — what is definitively supported by the evidence
9. **Limitations** — what the experiment does *not* claim
10. **Future Work** — possible research directions
