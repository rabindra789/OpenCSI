# Changelog

All notable changes to OpenCSI will be documented in this file.

## v0.2.0 — Signal Acquisition & Characterization

This release marks the completion of the Signal Acquisition & Characterization phase.
Four experiments characterize the ESP32 CSI acquisition pipeline, network traffic effects, and environmental sensitivity.

### Added

- **EXP-002** — CSI Signal Stability Characterization: idle callback rate ~0.25/s, min interval ~81ms, first-callback latency 23-60s
- **EXP-003** — CSI Startup Timeline Decomposition: WiFi association + DHCP = 90-99% of startup time (4-56s), CSI enable ~8ms
- **EXP-004** — Traffic-Driven CSI Sampling Characterization: callback rate saturates at ~10/s (serial 115200 bottleneck), HT ratio determined by traffic rate (idle=0%, 1pkt/s=34%, 10pkt/s=97%)
- **EXP-005** — Static Environmental Variation: metal object near ESP32 drops RSSI by 5 dBm and callback rate by 73%; door position measurably affects callback rate
- 10-bin interval histogram firmware for callback timing distribution
- `tools/exp_runner.py` — coordinated capture + traffic stimulus
- `tools/udp_sender.py` — controlled UDP packet generation
- `tools/tcp_stimulus.py` — controlled TCP traffic generation
- `tools/capture.py` — serial capture with configurable duration
- `docs/SENSING-ROADMAP.md` — capability definitions, experiment mapping, and design principles

### Changed

- README.md rewritten with experiment table and roadmap
- ROADMAP.md updated through v1.0.0
- Experiment registry expanded through EXP-006

### Fixed

- CSI callbacks no longer continue after experiment timer expires (`experiment_active` flag)

### Engineering Knowledge Gained

1. The CSI subsystem has a minimum callback interval of ~81 ms, likely a hardware/software limit (EXP-002).
2. WiFi association retries dominate startup time (2-30 disconnect/reconnect events), not CSI initialization (EXP-003).
3. Serial output at 115200 baud limits sustainable callback rate to ~10/s. Higher rates require higher baud rates or binary transport (EXP-004).
4. HT vs non-HT packet ratio is determined by WiFi rate adaptation, not by CSI configuration. Traffic rate drives HT adoption (EXP-004).
5. RSSI is stable within ±5 dBm and is traffic-independent, but responds to large environmental changes (EXP-005).
6. Callback rate depends on ambient packet arrival rate, making it an unreliable environmental sensor (EXP-005).

### Framework Status

- Firmware framework (WiFi, CSI, Transport): **stable, frozen at v0.1.1**
- All experiments use the same framework with zero modifications
- Framework changes will only occur if genuine engineering needs arise

---

## v0.1.1 — Data Collection Features

### Added

- `transport_begin()` — prints experiment metadata and CSV column header at capture start
- `transport_end()` — prints run completion marker
- Per-record timestamp (`esp_timer_get_time()` at callback) in CSV output
- Per-record sequence number in CSV output
- Self-describing CSV header row for machine-parseable output

### Changed

- `transport_write_csi_record()` signature updated: accepts `seq_num` and `timestamp_us` parameters
- EXP-001 firmware updated to use new transport API

---

## v0.1.0 — Foundation

### Added

- Development environment documentation for Windows + ESP-IDF v5.3.2
- Hardware verification workflow (M0)
- Raw CSI observation firmware (M1)
- Reusable firmware framework with three components:
  - **WiFi** — station init, connect, reconnect
  - **CSI** — capture configuration and callback registration
  - **Transport** — CSI record output over UART
- Published experiment: EXP-001 (CSI Callback Source Characterization)
- Experiment registry with lifecycle framework
- Framework architecture documentation (FRAMEWORK.md)
- Repository structure cleanup (root .gitignore, sdkconfig untracked)
- Writing style guidelines applied across all documentation

### Changed

- M1 firmware: LED blink task stack size fix (1024 -> 2048 words)

### Fixed

- `firmware/m0_minimal/sdkconfig` removed from git tracking

### Known Gaps

- No signal processing
- No presence detection
- No activity recognition
- No visualization tools
- No machine learning
- No multi-device sensing
- Single platform (ESP32 DevKit V1, Windows-only setup guide)
