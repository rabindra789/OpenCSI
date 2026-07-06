#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "wifi.h"
#include "csi.h"
#include "transport.h"

#define TAG "EXP_001"
#define LED_GPIO GPIO_NUM_2

typedef struct {
    uint32_t total_count;
    int8_t rssi_min;
    int8_t rssi_max;
    uint32_t sig_mode_count[4];
    uint32_t rate_count[256];
    uint32_t ant_count[2];
    int64_t started_us;
    int64_t first_csi_us;
    int64_t last_csi_us;
} csi_stats_t;

static csi_stats_t stats = { 0 };
static bool experiment_done = false;

static void print_stats(int64_t elapsed_us, bool final)
{
    double elapsed_s = (double)elapsed_us / 1e6;
    double avg = (elapsed_s > 0) ? (double)stats.total_count / elapsed_s : 0.0;

    printf("\n=== %s STATS [%s] ===\n", final ? "FINAL" : "INTERIM", CONFIG_EXP_SCENARIO_LABEL);
    printf("elapsed_s: %.1f\n", elapsed_s);
    printf("total_csi: %lu\n", (unsigned long)stats.total_count);
    printf("avg_per_sec: %.2f\n", avg);
    printf("rssi_range: %d .. %d dBm\n", stats.rssi_min, stats.rssi_max);

    printf("sig_mode: ");
    for (int i = 0; i < 4; i++) {
        if (stats.sig_mode_count[i] > 0)
            printf("%d=%lu ", i, (unsigned long)stats.sig_mode_count[i]);
    }
    printf("\n");

    printf("ant: ant0=%lu ant1=%lu\n",
           (unsigned long)stats.ant_count[0],
           (unsigned long)stats.ant_count[1]);

    printf("rate_dist: ");
    for (int i = 0; i < 256; i++) {
        if (stats.rate_count[i] > 0)
            printf("%d=%lu ", i, (unsigned long)stats.rate_count[i]);
    }
    printf("\n");

    if (stats.first_csi_us > 0) {
        double first_offset = (double)(stats.first_csi_us - stats.started_us) / 1e6;
        printf("first_csi_at: %.3f s\n", first_offset);
    }
    if (stats.last_csi_us > 0) {
        double last_offset = (double)(stats.last_csi_us - stats.started_us) / 1e6;
        printf("last_csi_at: %.3f s\n", last_offset);
    }
    printf("========================\n");
}

static void update_stats(wifi_csi_info_t *data)
{
    stats.total_count++;
    if (stats.total_count == 1) {
        stats.first_csi_us = esp_timer_get_time();
        stats.rssi_min = data->rx_ctrl.rssi;
        stats.rssi_max = data->rx_ctrl.rssi;
    } else {
        if (data->rx_ctrl.rssi < stats.rssi_min)
            stats.rssi_min = data->rx_ctrl.rssi;
        if (data->rx_ctrl.rssi > stats.rssi_max)
            stats.rssi_max = data->rx_ctrl.rssi;
    }
    stats.last_csi_us = esp_timer_get_time();

    int sm = data->rx_ctrl.sig_mode;
    if (sm >= 0 && sm < 4)
        stats.sig_mode_count[sm]++;

    int r = data->rx_ctrl.rate;
    if (r >= 0 && r < 256)
        stats.rate_count[r]++;

    int a = data->rx_ctrl.ant;
    if (a >= 0 && a < 2)
        stats.ant_count[a]++;
}

static void csi_callback(void *ctx, wifi_csi_info_t *data)
{
    transport_write_csi_record(data, stats.total_count);
    update_stats(data);
}

static void on_wifi_connected(void)
{
    csi_init(csi_callback);
}

static void blink_task(void *pvParameter)
{
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    while (1) {
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void stats_task(void *pvParameter)
{
    const int duration_s = CONFIG_EXP_DURATION_SECONDS;
    const TickType_t interval = pdMS_TO_TICKS(10000);

    vTaskDelay(pdMS_TO_TICKS(5000));

    while (1) {
        int64_t now = esp_timer_get_time();
        int64_t elapsed = now - stats.started_us;
        double elapsed_s = (double)elapsed / 1e6;

        if (elapsed_s >= duration_s && !experiment_done) {
            experiment_done = true;
            print_stats(elapsed, true);
            printf("=== EXP-001 COMPLETE [%s] ===\n", CONFIG_EXP_SCENARIO_LABEL);
        }

        if (!experiment_done) {
            if (stats.total_count > 0)
                print_stats(elapsed, false);
            else
                printf("[STATS] No CSI callbacks yet (%.0f s elapsed)\n", elapsed_s);
        }

        if (experiment_done)
            break;

        vTaskDelay(interval);
    }

    while (1) {
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(200));
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main(void)
{
    xTaskCreate(blink_task, "blink", 2048, NULL, 1, NULL);

    wifi_init(on_wifi_connected);

    stats.started_us = esp_timer_get_time();

    printf("=== EXP-001 START [%s] duration=%ds ===\n",
           CONFIG_EXP_SCENARIO_LABEL, CONFIG_EXP_DURATION_SECONDS);
    printf("WiFi connecting to SSID: %s\n", CONFIG_ESP_WIFI_SSID);

    xTaskCreate(stats_task, "stats", 4096, NULL, 2, NULL);
}
