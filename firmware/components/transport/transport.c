#include <stdio.h>
#include "transport.h"

void transport_write_csi_record(wifi_csi_info_t *data, uint32_t total_count)
{
    printf("\n=== CSI ===\n");
    printf("MAC (src): %02x:%02x:%02x:%02x:%02x:%02x\n",
           data->mac[0], data->mac[1], data->mac[2],
           data->mac[3], data->mac[4], data->mac[5]);
    printf("MAC (dst): %02x:%02x:%02x:%02x:%02x:%02x\n",
           data->dmac[0], data->dmac[1], data->dmac[2],
           data->dmac[3], data->dmac[4], data->dmac[5]);
    printf("first_word_invalid: %d\n", data->first_word_invalid);
    printf("len: %u\n", data->len);
    printf("rx_seq: %u\n", data->rx_seq);
    printf("payload_len: %u\n", data->payload_len);

    printf("rx_ctrl:\n");
    printf("  rssi: %d dBm\n",          data->rx_ctrl.rssi);
    printf("  rate: %u\n",              data->rx_ctrl.rate);
    printf("  sig_mode: %u (0=non-HT, 1=HT, 3=VHT)\n", data->rx_ctrl.sig_mode);
    printf("  mcs: %u\n",               data->rx_ctrl.mcs);
    printf("  cwb: %u (0=20MHz, 1=40MHz)\n", data->rx_ctrl.cwb);
    printf("  smoothing: %u\n",         data->rx_ctrl.smoothing);
    printf("  not_sounding: %u\n",      data->rx_ctrl.not_sounding);
    printf("  aggregation: %u\n",       data->rx_ctrl.aggregation);
    printf("  stbc: %u\n",              data->rx_ctrl.stbc);
    printf("  fec_coding: %u\n",        data->rx_ctrl.fec_coding);
    printf("  sgi: %u (0=Long GI, 1=Short GI)\n", data->rx_ctrl.sgi);
    printf("  noise_floor: %d dBm\n",   data->rx_ctrl.noise_floor);
    printf("  ampdu_cnt: %u\n",         data->rx_ctrl.ampdu_cnt);
    printf("  channel: %u\n",           data->rx_ctrl.channel);
    printf("  secondary_channel: %u (0=none, 1=above, 2=below)\n", data->rx_ctrl.secondary_channel);
    printf("  timestamp: %u us\n",      data->rx_ctrl.timestamp);
    printf("  ant: %u (0=ant0, 1=ant1)\n", data->rx_ctrl.ant);
    printf("  sig_len: %u\n",           data->rx_ctrl.sig_len);
    printf("  rx_state: %u\n",          data->rx_ctrl.rx_state);

    printf("CSI BUF (hex, first %u bytes):\n", data->len < 128 ? data->len : 128);
    for (uint16_t i = 0; i < data->len && i < 128; i++) {
        printf("%02x%c", (uint8_t)data->buf[i], (i % 32 == 31) ? '\n' : ' ');
    }
    if (data->len < 128) printf("\n");
    printf("============\n");

    printf("CSV,%lu,%u,%d,%u,%u,%u,%u,%u,%d,%u,%u,%u\n",
           (unsigned long)total_count,
           data->rx_seq,
           data->rx_ctrl.rssi,
           data->rx_ctrl.rate,
           data->rx_ctrl.sig_mode,
           data->rx_ctrl.mcs,
           data->rx_ctrl.cwb,
           data->rx_ctrl.ant,
           data->rx_ctrl.noise_floor,
           data->rx_ctrl.channel,
           data->rx_ctrl.timestamp,
           data->len);
}
