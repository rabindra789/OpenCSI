# OpenCSI Firmware Framework

This document explains how the firmware framework works, why it is structured the way it is, and how experiments use it.

---

## Why the framework exists

The first three experiments (M1, M2, EXP-001) each copied firmware from the previous one. Every new experiment meant duplicating code. We wanted experiments to focus on their research question, not on setting up WiFi and CSI.

The framework exists to provide reusable infrastructure. Experiments call into the framework. The framework does not call into experiments.

---

## How experiments use it

An experiment is an ESP-IDF project with its own `main.c`. It links against the framework components and adds only experiment-specific logic.

```
experiment/
  main/main.c         ← experiment logic (wires framework modules together)
  CMakeLists.txt      ← requires wifi csi transport
  Kconfig.projbuild   ← experiment-specific config (scenario label, duration)
```

The experiment's `main.c` follows this pattern:

1. Call `wifi_init()` with a callback.
2. When the callback fires (WiFi connected), call `csi_init()` with your CSI callback.
3. In the CSI callback, call `transport_write_csi_record()` then do your experiment-specific processing.
4. Run your experiment logic (timing, stimulus, aggregation).

The framework does not define how experiments should behave. It provides the building blocks.

---

## Three components, three responsibilities

### WiFi

**Owns:** Initializing the WiFi station, connecting to the configured AP, reconnecting on disconnection, and notifying the application when an IP address is obtained.

**Does not own:** CSI configuration, data processing, or transport. The WiFi component does not know that CSI exists.

**How experiments use it:**

```c
#include "wifi.h"

static void on_wifi_connected(void)
{
    // WiFi is ready. Enable CSI, start stimulus, etc.
}

void app_main(void)
{
    wifi_init(on_wifi_connected);
}
```

The only interface is `wifi_init()`. It takes a callback that fires once when the ESP32 obtains an IP address. After that, the WiFi component handles reconnection internally.

**Design decision:** The callback is a single function pointer, not an event system. Most experiments only need to react to "WiFi is ready." More complex event handling can be added by the experiment if needed.

---

### CSI

**Owns:** Configuring the ESP32 WiFi CSI subsystem (LTF settings, dump_ack, etc.), registering the application's callback, and enabling/disabling CSI capture.

**Does not own:** Filtering, processing, or interpreting CSI data. The CSI component does not look at the data it captures. It passes the raw `wifi_csi_info_t` record to the application's callback and does nothing else.

**How experiments use it:**

```c
#include "csi.h"

static void csi_callback(void *ctx, wifi_csi_info_t *data)
{
    // Raw CSI record received. Write it, process it, or store it.
}

static void on_wifi_connected(void)
{
    csi_init(csi_callback);
}
```

The CSI configuration is currently hardcoded (lltf_en, htltf_en, dump_ack_en, etc.). These values came from M1, where they were tested on real hardware. If an experiment needs different CSI parameters, the config must be changed in `csi.c`. This is intentional — CSI parameters should change only when an experiment specifically requires it.

**Design decision:** The CSI config is hardcoded, not configurable via Kconfig. This is because the CSI config values interact with each other in complex ways. Making them all Kconfig options would create a false sense of generality. When an experiment genuinely needs different CSI parameters, the config is changed in `csi.c` and verified.

---

### Transport

**Owns:** Moving CSI records off the board. Currently writes to UART via `printf`.

**Does not own:** The format of the record, or what happens to the data after it leaves the board. The transport module does not know what the data means.

**How experiments use it:**

```c
#include "transport.h"

static void csi_callback(void *ctx, wifi_csi_info_t *data)
{
    transport_write_csi_record(data, total_count);
}
```

The transport module exposes one function that writes the full CSI record (field dump + CSV line) in the same format used by EXP-001.

**Why transport is not called UART:** The current implementation uses UART, but the interface is independent of the medium. A future transport could write to USB CDC, TCP, UDP, or SD card without changing the rest of the framework. The experiment calls `transport_write_csi_record()` and does not need to know which medium is active.

**How to add a new transport:**

1. Create a new implementation file (e.g., `transport_tcp.c`).
2. Implement `transport_write_csi_record()` using the new medium.
3. Swap the file in `CMakeLists.txt`.

No other module changes. The CSI component does not know about transport. The experiment does not know about transport. The transport is a pluggable output path.

**Design decision:** The transport function currently packages field dumps and CSV together because that is what the experiments needed. A future experiment that needs a different output format can either replace `transport_write_csi_record()` or call `printf` directly. The framework should not guess what output formats experiments will need.

---

## Why responsibilities are intentionally small

Each component does one thing and stops:

- WiFi connects. It does not enable CSI.
- CSI captures. It does not interpret data.
- Transport outputs. It does not decide what to output.

Small responsibilities mean:
- You can understand a component by reading one file.
- You can replace a component without touching the others.
- You can test a component in isolation.

---

## Why experiments contain research logic

The framework contains reusable infrastructure. The experiment contains everything specific to its research question: what data to collect, how to aggregate it, when to stop, what stimulus to apply.

This separation means:
- Experiments can change without changing the framework.
- The framework can improve without changing past experiments.
- Past experiments remain reproducible because their firmware is frozen.

---

## Abstractions are extracted, not invented

The three components (WiFi, CSI, Transport) were not designed first and then implemented. They were extracted from working experiment code (EXP-001) after the experiments proved they worked.

This is a deliberate principle:

> Interfaces are discovered through implementation, not invented in advance.

We did not design a transport abstraction and then implement UART support. We moved the UART code into its own file and called it transport. If a future experiment needs TCP, the transport interface will evolve based on that genuine need — not based on what we guessed it might need.

This keeps the framework minimal. Every line of the framework was written because an experiment needed it.

---

## Kconfig organization

Each component exposes its own Kconfig:

| Component | Kconfig | Configures |
|-----------|---------|------------|
| WiFi | `ESP_WIFI_SSID`, `ESP_WIFI_PASSWORD` | Network credentials |
| Experiment | `EXP_SCENARIO_LABEL`, `EXP_DURATION_SECONDS` | Experiment parameters |

The CSI component does not expose Kconfig. Its parameters are hardcoded (see CSI section above).

The project-level `sdkconfig.defaults` provides values for both component and ESP-IDF-level options (including `CONFIG_ESP_WIFI_CSI_ENABLED` which must be set at the ESP-IDF level).

---

## Version 1

This framework is Version 1. It supports the CSI observation workflow that was validated through EXP-001.

Future changes should happen only when:
- A new experiment reveals a genuine need that the framework cannot support.
- A bug is discovered.
- The framework becomes unnecessarily difficult to use.

We should not redesign the framework because we have new ideas. New ideas should be tested through experiments first. If multiple experiments need the same interface, that interface can be extracted.
