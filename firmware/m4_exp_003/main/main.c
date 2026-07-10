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

#define LED_GPIO GPIO_NUM_2

typedef struct {
    int64_t t_app_main;
    int64_t t_wifi_init_done;
    int64_t t_got_ip;
    int64_t t_csi_enabled;
    int64_t t_first_csi;
    bool    got_first_csi;
    int64_t started_us;
} startup_timeline_t;

static startup_timeline_t tl = { 0 };
static bool experiment_done = false;

static void print_timeline(void)
{
    int64_t base = tl.t_app_main;
    int64_t t_wifi_init_done = tl.t_wifi_init_done;
    int64_t t_got_ip        = tl.t_got_ip;
    int64_t t_csi_enabled   = tl.t_csi_enabled;
    int64_t t_first_csi     = tl.t_first_csi;

    int64_t d_init    = t_wifi_init_done - base;
    int64_t d_connect = t_got_ip - t_wifi_init_done;
    int64_t d_csi     = t_csi_enabled - t_got_ip;
    int64_t d_wait    = t_first_csi - t_csi_enabled;
    int64_t total     = t_first_csi - base;

    printf("\n=== STARTUP TIMELINE ===\n");
    printf("app_main      : %10lld us\n", 0LL);
    printf("wifi_init done: %10lld us  (+%lld us)\n",
           (long long)d_init, (long long)d_init);
    printf("IP acquired   : %10lld us  (+%lld us)\n",
           (long long)(t_got_ip - base), (long long)d_connect);
    printf("CSI enabled   : %10lld us  (+%lld us)\n",
           (long long)(t_csi_enabled - base), (long long)d_csi);
    printf("first CSI     : %10lld us  (+%lld us)\n",
           (long long)(t_first_csi - base), (long long)d_wait);
    printf("---\n");
    printf("Total startup : %.3f s (%lld us)\n",
           (double)total / 1e6, (long long)total);
    printf("Breakdown:\n");
    printf("  wifi_init_call:  %5.1f%%  (%lld us)\n",
           (double)d_init / total * 100.0, (long long)d_init);
    printf("  connect+DHCP:    %5.1f%%  (%lld us)\n",
           (double)d_connect / total * 100.0, (long long)d_connect);
    printf("  csi_enable:      %5.1f%%  (%lld us)\n",
           (double)d_csi / total * 100.0, (long long)d_csi);
    printf("  wait_first_csi:  %5.1f%%  (%lld us)\n",
           (double)d_wait / total * 100.0, (long long)d_wait);
    printf("========================\n");
}

static void print_stats(int64_t elapsed_us, bool final, uint32_t total_count,
                         int8_t rssi_min, int8_t rssi_max)
{
    double elapsed_s = (double)elapsed_us / 1e6;
    double avg = (elapsed_s > 0) ? (double)total_count / elapsed_s : 0.0;

    printf("\n=== %s STATS ===\n", final ? "FINAL" : "INTERIM");
    printf("elapsed_s: %.1f\n", elapsed_s);
    printf("total_csi: %lu\n", (unsigned long)total_count);
    printf("avg_per_sec: %.3f\n", avg);
    printf("rssi_range: %d .. %d dBm\n", rssi_min, rssi_max);
    printf("========================\n");
}

static void csi_callback(void *ctx, wifi_csi_info_t *data)
{
    int64_t now = esp_timer_get_time();

    if (!tl.got_first_csi) {
        tl.got_first_csi = true;
        tl.t_first_csi = now;
        print_timeline();
    }

    transport_write_csi_record(data, 0, now);
}

static void on_wifi_connected(void)
{
    tl.t_got_ip = esp_timer_get_time();
    printf("[TIMELINE] IP acquired at %lld us\n",
           (long long)(tl.t_got_ip - tl.t_app_main));

    printf("[TIMELINE] Enabling CSI...\n");
    csi_init(csi_callback);
    tl.t_csi_enabled = esp_timer_get_time();
    printf("[TIMELINE] CSI enabled at %lld us\n",
           (long long)(tl.t_csi_enabled - tl.t_app_main));

    printf("[TIMELINE] Startup instrumentation complete. Waiting for first CSI callback...\n");
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

    tl.started_us = esp_timer_get_time();

    while (1) {
        int64_t now = esp_timer_get_time();
        int64_t elapsed = now - tl.started_us;
        double elapsed_s = (double)elapsed / 1e6;

        if (elapsed_s >= duration_s && !experiment_done) {
            experiment_done = true;
            print_stats(elapsed, true, 0, 0, 0);
            transport_end();
            printf("=== EXP-003 COMPLETE ===\n");
        }

        if (!experiment_done) {
            printf("[STATS] %.0f s elapsed\n", elapsed_s);
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
    tl.t_app_main = esp_timer_get_time();
    printf("=== EXP-003 START ===\n");
    printf("[TIMELINE] app_main entry at %lld us\n", 0LL);

    xTaskCreate(blink_task, "blink", 2048, NULL, 1, NULL);

    wifi_init(on_wifi_connected);
    tl.t_wifi_init_done = esp_timer_get_time();
    printf("[TIMELINE] wifi_init returned at %lld us\n",
           (long long)(tl.t_wifi_init_done - tl.t_app_main));

    transport_begin("EXP-003", "startup", CONFIG_EXP_DURATION_SECONDS);

    printf("WiFi connecting to SSID: %s\n", CONFIG_ESP_WIFI_SSID);

    xTaskCreate(stats_task, "stats", 4096, NULL, 2, NULL);
}
