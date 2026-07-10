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

#define HISTOGRAM_BINS 10

static const uint64_t histogram_limits[HISTOGRAM_BINS] = {
    50000,      /* 0-50 ms */
    100000,     /* 50-100 ms */
    500000,     /* 100-500 ms */
    1000000,    /* 0.5-1 s */
    2000000,    /* 1-2 s */
    5000000,    /* 2-5 s */
    10000000,   /* 5-10 s */
    30000000,   /* 10-30 s */
    60000000,   /* 30-60 s */
    UINT64_MAX, /* >60 s */
};

static const char *histogram_labels[HISTOGRAM_BINS] = {
    "<50ms", "50-100ms", "100-500ms", "0.5-1s", "1-2s",
    "2-5s", "5-10s", "10-30s", "30-60s", ">60s"
};

typedef struct {
    uint32_t total_count;
    int8_t rssi_min;
    int8_t rssi_max;
    uint32_t sig_mode_count[4];
    uint32_t rate_count[256];
    uint32_t ant_count[2];
    uint32_t histogram[HISTOGRAM_BINS];
    int64_t started_us;
    int64_t prev_csi_us;
    bool    first_callback;
    bool    experiment_active;
} csi_stats_t;

static csi_stats_t stats = { 0 };
static bool experiment_done = false;

static void init_stats(void)
{
    memset(&stats, 0, sizeof(stats));
    stats.rssi_min = 127;
    stats.rssi_max = -128;
    stats.first_callback = true;
}

static void update_histogram(int64_t interval_us)
{
    uint64_t abs_interval = (interval_us >= 0) ? (uint64_t)interval_us : 0;
    for (int i = 0; i < HISTOGRAM_BINS; i++) {
        if (abs_interval < histogram_limits[i]) {
            stats.histogram[i]++;
            return;
        }
    }
}

static void print_stats(int64_t elapsed_us, bool final)
{
    double elapsed_s = (double)elapsed_us / 1e6;
    double avg_rate = (elapsed_s > 0) ? (double)stats.total_count / elapsed_s : 0.0;

    printf("\n=== %s STATS [%s] ===\n", final ? "FINAL" : "INTERIM", CONFIG_EXP_SCENARIO_LABEL);
    printf("elapsed_s: %.1f\n", elapsed_s);
    printf("total_csi: %lu\n", (unsigned long)stats.total_count);
    printf("avg_per_sec: %.3f\n", avg_rate);

    if (stats.rssi_min <= stats.rssi_max)
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

    uint32_t hist_total = 0;
    for (int i = 0; i < HISTOGRAM_BINS; i++)
        hist_total += stats.histogram[i];

    printf("interval_hist (%lu samples):\n", (unsigned long)hist_total);
    for (int i = 0; i < HISTOGRAM_BINS; i++) {
        if (stats.histogram[i] > 0) {
            double pct = hist_total > 0 ? (double)stats.histogram[i] / hist_total * 100.0 : 0.0;
            printf("  %s: %lu (%.1f%%)\n",
                   histogram_labels[i],
                   (unsigned long)stats.histogram[i], pct);
        }
    }

    printf("========================\n");
}

static void csi_callback(void *ctx, wifi_csi_info_t *data)
{
    int64_t now = esp_timer_get_time();

    if (stats.first_callback) {
        stats.started_us = now;
        stats.prev_csi_us = now;
        stats.first_callback = false;
        stats.experiment_active = true;
        printf("[EXP-004] First CSI callback. Experiment clock started.\n");
    }

    if (stats.experiment_active) {
        transport_write_csi_record(data, stats.total_count, now);

        int64_t interval = now - stats.prev_csi_us;
        stats.prev_csi_us = now;

        stats.total_count++;
        update_histogram(interval);

        if (data->rx_ctrl.rssi < stats.rssi_min)
            stats.rssi_min = data->rx_ctrl.rssi;
        if (data->rx_ctrl.rssi > stats.rssi_max)
            stats.rssi_max = data->rx_ctrl.rssi;

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
}

static void on_wifi_connected(void)
{
    printf("[EXP-004] WiFi connected. Enabling CSI...\n");
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
        if (experiment_done)
            break;

        if (!stats.experiment_active) {
            vTaskDelay(interval);
            continue;
        }

        int64_t now = esp_timer_get_time();
        int64_t elapsed = now - stats.started_us;
        double elapsed_s = (double)elapsed / 1e6;

        if (elapsed_s >= duration_s && !experiment_done) {
            experiment_done = true;
            stats.experiment_active = false;
            print_stats(elapsed, true);
            transport_end();
            printf("=== EXP-004 COMPLETE [%s] ===\n", CONFIG_EXP_SCENARIO_LABEL);
            break;
        }

        if (stats.total_count > 0)
            print_stats(elapsed, false);
        else
            printf("[STATS] No CSI callbacks yet (%.0f s elapsed)\n", elapsed_s);

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

#ifdef CONFIG_EXP_ENABLE_TCP_SERVER
#include "lwip/sockets.h"
#include "lwip/netdb.h"

static void tcp_server_task(void *pvParameter)
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_fd < 0) {
        printf("[TCP] socket() failed\n");
        vTaskDelete(NULL);
        return;
    }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(8888),
        .sin_addr = { INADDR_ANY },
    };

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("[TCP] bind() failed\n");
        close(listen_fd);
        vTaskDelete(NULL);
        return;
    }

    listen(listen_fd, 1);
    printf("[TCP] Listening on port 8888...\n");

    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int sock = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);
    if (sock < 0) {
        printf("[TCP] accept() failed\n");
        close(listen_fd);
        vTaskDelete(NULL);
        return;
    }

    char client_ip[16];
    inet_ntoa_r(client_addr.sin_addr, client_ip, sizeof(client_ip));
    printf("[TCP] Accepted connection from %s\n", client_ip);

    char buf[256];
    while (!experiment_done) {
        int n = read(sock, buf, sizeof(buf));
        if (n <= 0)
            break;
        write(sock, buf, n);
    }

    close(sock);
    close(listen_fd);
    printf("[TCP] Server stopped\n");
    vTaskDelete(NULL);
}
#endif

void app_main(void)
{
    printf("=== EXP-004 START [%s] ===\n", CONFIG_EXP_SCENARIO_LABEL);

    xTaskCreate(blink_task, "blink", 2048, NULL, 1, NULL);

    init_stats();

    wifi_init(on_wifi_connected);

    transport_begin("EXP-004", CONFIG_EXP_SCENARIO_LABEL, CONFIG_EXP_DURATION_SECONDS);

    printf("WiFi connecting to SSID: %s\n", CONFIG_ESP_WIFI_SSID);

#ifdef CONFIG_EXP_ENABLE_TCP_SERVER
    xTaskCreate(tcp_server_task, "tcp_server", 4096, NULL, 2, NULL);
#endif

    xTaskCreate(stats_task, "stats", 4096, NULL, 2, NULL);
}
