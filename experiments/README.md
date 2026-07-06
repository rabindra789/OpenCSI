# OpenCSI Experiment Registry

Each experiment is a self-contained, reproducible investigation of the ESP32 CSI subsystem.

## Structure

```
experiments/
├── README.md              This file — central registry index
├── EXP-001/
│   ├── report.md          Published experiment report
│   └── data/              Raw evidence, organized by scenario
├── EXP-002/
│   ├── objective.md       Planned experiment design
│   └── data/              (populated during execution)
└── EXP-003/               (proposed, not yet started)
```

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

## Registry

| ID | Title | Status | Result |
|----|-------|--------|--------|
| [EXP-001](EXP-001/report.md) | CSI Callback Source Characterization | **Published** | CSI callbacks increased when packets were addressed to ESP32; traffic between other devices did not produce a comparable increase |
| EXP-002 | CSI Callback Rate Characterization | **Planned** | — |
| EXP-003 | *(available)* | Proposed | — |
| EXP-004 | *(available)* | Proposed | — |
| EXP-005 | *(available)* | Proposed | — |
