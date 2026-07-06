#include <stdio.h>
#include "esp_wifi.h"
#include "csi.h"

static wifi_csi_config_t csi_config = {
    .lltf_en = true,
    .htltf_en = true,
    .stbc_htltf2_en = true,
    .ltf_merge_en = true,
    .channel_filter_en = true,
    .manu_scale = true,
    .shift = 0,
    .dump_ack_en = true,
};

void csi_init(csi_callback_t callback)
{
    printf("Enabling CSI...\n");
    esp_wifi_set_csi_config(&csi_config);
    esp_wifi_set_csi_rx_cb(callback, NULL);
    esp_wifi_set_csi(true);
    printf("CSI enabled.\n");
}
