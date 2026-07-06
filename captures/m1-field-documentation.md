# M1 — CSI Data Field Documentation

Observed on ESP32-D0WD-V3 rev3.0, ESP-IDF v5.3.2
WiFi: Station mode, connected to 2.4GHz AP (WPA2-PSK, channel 1, 20MHz)

## wifi_csi_info_t

| Field              | Type      | Observed Value    | Notes |
|--------------------|-----------|-------------------|-------|
| `mac`              | uint8_t[6]| `00:00:00:00:00:00` | Source MAC — all zeros observed in every packet. May be a hardware limitation for non-HT/ACK frames. |
| `dmac`             | uint8_t[6]| `a8:42:e3:5b:f4:20` | Destination MAC — always our ESP32's own station MAC. |
| `first_word_invalid` | bool    | `1`               | First 4 bytes of CSI buf are invalid (hardware limitation). Always observed as true. |
| `len`              | uint16_t  | `128`             | Total CSI data length in bytes. Always 128 for HT20 packets. |
| `rx_seq`           | uint16_t  | `0, 4, 1763`      | RX sequence number. Ranges from 0 to 1763 depending on session. |
| `payload_len`      | uint16_t  | `14`              | WiFi payload length in bytes. Very small — consistent with NULL data frames or ACKs. |
| `buf`              | int8_t*   | (see below)       | CSI channel estimates. Signed 8-bit I/Q pairs. 128 bytes = 64 subcarriers × 2 bytes (I+Q). |
| `hdr`              | uint8_t*  | (not dumped)      | Raw WiFi packet header. |
| `payload`          | uint8_t*  | (not dumped)      | Raw WiFi packet payload. |

## wifi_pkt_rx_ctrl_t

| Field              | Type    | Size | Observed       | Notes |
|--------------------|---------|------|----------------|-------|
| `rssi`             | int8_t  | 8b   | -77 to -83 dBm | Received signal strength. Negative dBm values. |
| `rate`             | uint8_t | 5b   | 11             | PHY rate encoding. Value 11 = 11 Mbps (802.11b long preamble). Only valid for non-HT packets. |
| `sig_mode`         | uint8_t | 2b   | 0              | Protocol: 0=non-HT (802.11b/g), 1=HT (802.11n), 3=VHT (802.11ac). All observed packets are non-HT. |
| `mcs`              | uint8_t | 7b   | 0              | Modulation Coding Scheme. 0 for non-HT packets. |
| `cwb`              | uint8_t | 1b   | 0              | Channel bandwidth: 0=20MHz, 1=40MHz. All packets at 20MHz. |
| `smoothing`        | uint8_t | 1b   | 0              | Channel estimate smoothing recommended. |
| `not_sounding`     | uint8_t | 1b   | 0              | 0 = PPDU is a sounding PPDU (for channel estimation). |
| `aggregation`      | uint8_t | 1b   | 0              | 0 = MPDU (non-aggregated), 1 = AMPDU. |
| `stbc`             | uint8_t | 2b   | 0              | Space Time Block Coding. 0 = non-STBC. |
| `fec_coding`       | uint8_t | 1b   | 0              | Forward Error Correction. 1 = LDPC for 11n packets. |
| `sgi`              | uint8_t | 1b   | 0              | Guard Interval: 0=Long GI (800ns), 1=Short GI (400ns). |
| `noise_floor`      | int8_t  | 8b   | -97 dBm        | RF noise floor. Consistent across all observations. |
| `ampdu_cnt`        | uint8_t | 8b   | 0              | AMPDU subframe count. 0 = not aggregated. |
| `channel`          | uint8_t | 4b   | 1              | Primary channel number. |
| `secondary_channel`| uint8_t | 4b   | 0              | Secondary channel: 0=none, 1=above, 2=below. |
| `timestamp`        | uint32_t| 32b  | 8M-22M us      | Local RX timestamp in microseconds. |
| `ant`              | uint8_t | 1b   | 0              | Receiving antenna: 0=antenna 0, 1=antenna 1. |
| `sig_len`          | uint16_t| 12b  | 14             | Packet length including FCS. 14 bytes is too small for a data frame — likely ACK or NULL func. |
| `rx_state`         | uint8_t | 8b   | 0              | 0 = no error. Non-zero = error code. |

## CSI Buffer Format (128 bytes)

The CSI buffer contains channel estimates as signed 8-bit I/Q pairs:

- **Bytes 0-3**: First word (invalid when `first_word_invalid == 1`). Observed as `0e e0 00 00` consistently.
- **Bytes 4-127**: 62 subcarrier pairs × 2 bytes (I then Q) = 124 bytes. Plus zero padding at DC subcarrier.

The buffer effectively contains 62 valid subcarriers (out of 64 total for HT20), with the DC subcarrier (index 0) and one edge subcarrier zeroed out.

For non-HT (legacy) packets, the CSI data comes from the Legacy Long Training Field (L-LTF), which includes 52 used subcarriers (-26 to -1, 1 to 26) in 20MHz OFDM, or from DSSS preamble for 802.11b packets.

## Key Findings

1. **All observed CSI packets are non-HT (802.11b/g) frames at rate 11 Mbps.** No HT (802.11n) packets were captured via CSI, likely because the ESP32's WiFi driver prioritizes CSI capture for certain packet types, or the AP's HT traffic doesn't reliably trigger the CSI callback.

2. **The source MAC is always zero** (`00:00:00:00:00:00`). This may be a known hardware limitation — the CSI callback may not populate the source MAC correctly for certain frame types (ACKs, NULL func frames).

3. **Packet rate is low** — approximately 1-3 CSI-triggering packets per second. This suggests only specific frame types (possibly ACKs sent by the ESP32 itself, or and/or NULL function data frames from the AP) pass through the CSI pipeline.

4. **CSI data length is fixed at 128 bytes** for this configuration (HT20, sig_mode=0). The format is consistent: first 4 bytes invalid, then 62 subcarrier I/Q pairs with some zero padding.

5. **RSSI varies by a few dB** (-77 to -83 dBm) across samples, while noise floor is stable at -97 dBm, giving an SNR of approximately 14-20 dB.

6. **`dump_ack_en` is enabled** — the CSI callback fires for ACK frames. The small payload_len (14 bytes) and sig_len (14 bytes) strongly suggest these are ACK frames or NULL data frames.

## Recommendations for M2

- Investigate why no HT packets trigger CSI. May require setting a specific CSI config or ensuring the AP sends HT packets to the station.
- Consider adding promiscuous mode to capture CSI from all nearby traffic, not just frames addressed to the ESP32.
- The source MAC issue should be investigated — it might be resolved by filtering for specific frame types.
