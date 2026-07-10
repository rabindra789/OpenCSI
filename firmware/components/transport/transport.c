#include <stdio.h>
#include "transport.h"

static const char *s_exp_id = "";
static const char *s_scenario = "";

void transport_begin(const char *exp_id, const char *scenario, int duration_s)
{
    s_exp_id = exp_id;
    s_scenario = scenario;

    printf("=== %s START [%s] duration=%ds ===\n", exp_id, scenario, duration_s);
    printf("# columns: seq_num, timestamp_us, rx_seq, rssi_dbm, rate, "
           "sig_mode, mcs, cwb, ant, noise_floor_dbm, channel, "
           "wifi_timestamp_us, payload_len\n");
}

void transport_write_csi_record(wifi_csi_info_t *data, uint32_t seq_num, int64_t timestamp_us)
{
    printf("\n=== CSI [%lu] ===\n", (unsigned long)seq_num);
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

    printf("CSV,%lu,%lu,%u,%d,%u,%u,%u,%u,%u,%d,%u,%u,%u\n",
           (unsigned long)seq_num,
           (unsigned long)timestamp_us,
           (unsigned)data->rx_seq,
           (int)data->rx_ctrl.rssi,
           (unsigned)data->rx_ctrl.rate,
           (unsigned)data->rx_ctrl.sig_mode,
           (unsigned)data->rx_ctrl.mcs,
           (unsigned)data->rx_ctrl.cwb,
           (unsigned)data->rx_ctrl.ant,
           (int)data->rx_ctrl.noise_floor,
           (unsigned)data->rx_ctrl.channel,
           (unsigned)data->rx_ctrl.timestamp,
           (unsigned)data->len);
}

void transport_end(void)
{
    printf("=== %s COMPLETE [%s] ===\n", s_exp_id, s_scenario);
}
