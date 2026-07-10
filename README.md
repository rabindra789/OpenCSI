# OpenCSI

OpenCSI is a reliable, evidence-driven WiFi CSI human sensing framework for the ESP32.

## Experiments

| Experiment | Status | Summary |
|---|---|---|
| [EXP-001](experiments/EXP-001/report.md) | **Peer Reviewed** | CSI callbacks come from ACKs to packets addressed to ESP32; not from third-party traffic |
| [EXP-002](experiments/EXP-002/report.md) | **Reviewed** | Idle CSI callback rate ~0.25/s; min interval ~81ms; first-callback latency 23-60s |
| [EXP-003](experiments/EXP-003/report.md) | **Reviewed** | WiFi association + DHCP = 90-99% of startup time (4-56s); CSI enable takes ~8ms |
| [EXP-004](experiments/EXP-004/report.md) | **Analyzed** | Callback rate saturates at ~10/s (serial 115200 bottleneck); HT ratio determined by traffic rate |
| [EXP-005](experiments/EXP-005/report.md) | **Analyzed** | Metal object near ESP32 drops RSSI by 5 dBm; callback rate sensitive to door position |

Full experiment reports are in the [`experiments/`](experiments/) directory.

## Project Structure

```
experiments/         Peer-reviewed experiment reports, raw data, and analysis
firmware/            ESP-IDF firmware projects + reusable framework components
  components/        Reusable framework: wifi, csi, transport (v0.1.1, frozen)
  m1_exp_001/        EXP-001 firmware
  ...
tools/               Capture and analysis tools
docs/                Development environment and project documentation
```

See [`docs/DEVELOPMENT-ENVIRONMENT.md`](docs/DEVELOPMENT-ENVIRONMENT.md) for setup and [`docs/FRAMEWORK.md`](docs/FRAMEWORK.md) for architecture.

## Roadmap

| Phase | Version | Status |
|---|---|---|
| Foundation | v0.1.x | Complete |
| Hardware Characterization (EXP-001-003) | v0.2.0 | Complete |
| Signal Characterization (EXP-004-006) | v0.3.0 | Active |
| Motion Characterization | v0.4.0 | Next |
| Presence Detection | v0.5.0 | Planned |
| Activity Characterization | v0.6.0 | Planned |

See [`ROADMAP.md`](ROADMAP.md) for details.

## License

MIT

## License

MIT
