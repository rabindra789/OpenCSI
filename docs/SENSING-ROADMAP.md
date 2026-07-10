# Sensing Capability Roadmap

This document defines what human sensing capabilities OpenCSI aims to provide and how controlled experiments build toward them.

The roadmap separates **capability** (what we want to achieve) from **current implementation** (how we achieve it with today's hardware). As the project matures, implementations improve while capabilities remain stable.

---

## Capability definitions

### Motion Detection

**Question:** Is something moving in the sensing area?

**Definition:** Detect that the CSI signal is changing in a way consistent with human movement. This does not distinguish between a person walking, a pet moving, or a fan rotating. It detects that something changed.

**Why first:** Motion produces the largest and most reliable CSI variation. It is the easiest sensing capability to validate and the foundation for all later capabilities.

**Implementation bounds:** The initial version may only detect large movements (walking). Sensitivity improves through characterization experiments.

### Motion Characterization

**Question:** How does human movement affect the CSI signal?

**Definition:** Understand how different types of movement — speed, direction, distance — manifest in CSI amplitude and phase across subcarriers.

**Why second:** Before we quantify intensity, we need to understand the relationship between movement and signal. This is a research activity that feeds into Motion Detection.

### Presence Detection

**Question:** Can we reliably distinguish an occupied space from an empty one?

**Definition:** Determine whether a person is present within the sensing area, regardless of whether they are moving or stationary. This includes detection of a sitting or standing person who is not moving.

**Why third:** Presence is the hardest sensing capability because it must handle the stationary case. Breathing produces subtle CSI variation (0.2–0.5 Hz, subcarrier-level). It requires better signal processing than motion detection.

**Implementation bounds:** The initial version may achieve presence detection only during movement (motion-based presence). Full presence (including stationary) requires breathing detection or similar fine-grained analysis.

### Coarse Activity Recognition

**Question:** Can we distinguish different types of human activity?

**Definition:** Classify CSI patterns into activity categories such as walking, sitting, standing, or lying down.

**Why fourth:** Activity recognition builds on motion characterization and presence detection. It requires distinguishing patterns, not just detecting change.

**Implementation bounds:** Initial version targets 2–3 coarse categories (walking, stationary, transitional). Finer granularity follows.

---

## Cross-cutting research

These are research activities that support every sensing capability. They are not capabilities exposed by OpenCSI — they are the work required to build them.

### Environment Characterization

**Question:** What is the sensing environment doing?

**Definition:** Characterize the baseline CSI behavior of an environment — its noise floor, typical variation, multipath structure, and stability over time.

**Rationale:** Every capability depends on understanding the environment. Different rooms, APs, times of day, and furniture layouts produce different CSI baselines. Without characterization, we cannot distinguish signal from noise.

### Signal Characterization

**Question:** What does the CSI signal itself look like?

**Definition:** Measure and document the raw properties of the CSI stream — packet rate, callback timing, RSSI stability, noise floor, channel variation, CSI stability, subcarrier behavior.

**Rationale:** Before we can detect people or movement, we must understand the signal we are working with. This is the work already begun in EXP-001 and continuing in the next experiments.

### Benchmarking

**Question:** How well does the system perform?

**Definition:** Standardized metrics and procedures for evaluating sensing capabilities — accuracy, precision, recall, latency, false positive rate, and reliability across environments.

**Rationale:** Benchmarking is not a separate capability. It applies to every capability. A presence detector without benchmarks is a guess.

---

## Capability vs. implementation

OpenCSI separates the **what** from the **how**:

| Capability | Example initial implementation | Possible future implementation |
|------------|------------------------------|-------------------------------|
| Motion Detection | Variance threshold on raw RSSI | Subcarrier phase analysis |
| Motion Characterization | Walking vs. empty CSI comparison | Multi-subcarrier correlation |
| Presence Detection | Motion-based only | Breathing detection + motion |
| Activity Recognition | Walking vs. stationary | N-class classifier |

The capability definition does not change when the implementation improves. This keeps the roadmap stable while allowing engineering to evolve.

---

## The experimental path

Every capability is preceded by characterization and research. We do not build algorithms before we understand the signal.

```
v0.1.0  Foundation
         ↓
v0.2.0  Hardware Characterization
        What does the ESP32 CSI subsystem measure?
         ↓
v0.3.0  Signal Characterization
        What does the CSI signal look like?
         ↓
v0.4.0  Motion Characterization
        How does human movement affect CSI?
         ↓
v0.5.0  Presence Detection
        Can we tell if a person is present?
         ↓
v0.6.0  Activity Characterization
        Can we distinguish different activities?
```

Each version answers one question before the next version begins.

---

## Experiment-to-capability mapping

Every experiment connects to a specific capability. An experiment may support multiple capabilities but answers one primary question.

Experiment numbers are assigned sequentially as research questions arise. Examples below illustrate the kind of experiments each stage may include.

### Hardware Characterization (v0.2.0)

- What traffic generates CSI callbacks? (EXP-001 ✅)
- How does callback rate scale with traffic?
- How does CSI vary across channels?

### Signal Characterization (v0.3.0)

- What is the static environment baseline?
- How stable is CSI over hours?
- What is the noise floor?

### Motion Characterization (v0.4.0)

- How does walking affect CSI?
- What is the minimum detectable movement?
- How do different environments affect motion signatures?

### Presence Detection (v0.5.0)

- Can we detect a stationary person?
- How does body position affect detection?
- Multiple occupancy — one vs. two people?

### Activity Characterization (v0.6.0)

- Walking vs. sitting vs. standing
- Activity transitions
- Multi-subject scenarios

---

## Principles

1. **Characterize before detecting.** Every detection capability is preceded by at least one characterization experiment.

2. **One question per experiment.** An experiment answers one primary question. Ancillary observations are documented separately.

3. **Capabilities are independent of implementations.** Presence detection remains the goal even if the initial implementation is motion-based.

4. **Benchmarking is not optional.** Every detection capability must include a benchmarking experiment before it is considered validated.

5. **The roadmap guides experiments, not the other way around.** Experiments serve the roadmap. If an experiment suggests the roadmap is wrong, we adjust the roadmap — but we do not abandon it.

6. **Every capability should be independently useful.** Motion Detection should provide value even if Presence Detection does not exist yet. Presence Detection should provide value even if Activity Recognition has not been implemented. Each capability stands on its own.
