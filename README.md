# OpenCSI

CSI sensing on ESP32 — through controlled experiments, not guesswork.

## Research Progress

| Experiment | Status | Summary |
|---|---|---|
| [EXP-001](experiments/EXP-001/report.md) | **Published** | Characterized CSI callback behavior under different traffic scenarios |
| EXP-002 | Planned | CSI callback rate characterization |

Full experiment reports are available in the [`experiments/`](experiments/) directory.

## Project Structure

```
experiments/         Published experiment reports, raw data, and analysis
firmware/            ESP-IDF firmware projects for each milestone and experiment
captures/            Serial capture evidence files
docs/                Development environment and project documentation
```

See [`docs/DEVELOPMENT-ENVIRONMENT.md`](docs/DEVELOPMENT-ENVIRONMENT.md) for setup instructions.

## Milestones

| Milestone | Description | Status |
|---|---|---|
| M-1 | Toolchain verification | Complete |
| M0 | End-to-end hardware workflow | Complete |
| M1 | CSI observation firmware | Complete |

## License

MIT
