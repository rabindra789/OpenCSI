#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#define TAG "CSI_OBSERVE"

static void csi_callback(void *ctx, wifi_csi_info_t *data)
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
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        printf("WiFi started. Connecting...\n");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        printf("Disconnected. Reconnecting...\n");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        printf("Connected. IP: " IPSTR "\n", IP2STR(&event->ip_info.ip));
        printf("Enabling CSI...\n");

        wifi_csi_config_t csi_config = {
            .lltf_en = true,
            .htltf_en = true,
            .stbc_htltf2_en = true,
            .ltf_merge_en = true,
            .channel_filter_en = true,
            .manu_scale = true,
            .shift = 0,
            .dump_ack_en = true,
        };
        esp_wifi_set_csi_config(&csi_config);
        esp_wifi_set_csi_rx_cb(csi_callback, NULL);
        esp_wifi_set_csi(true);
        printf("CSI enabled. Observing...\n");
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .password = CONFIG_ESP_WIFI_PASSWORD,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    printf("WiFi connecting to SSID: %s\n", CONFIG_ESP_WIFI_SSID);
}
