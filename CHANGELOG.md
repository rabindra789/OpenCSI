# Changelog

All notable changes to OpenCSI will be documented in this file.

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
