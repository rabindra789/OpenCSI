#include <stdio.h>
#include "esp_chip_info.h"
#include "esp_mac.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

void app_main(void)
{
    printf("========== OpenCSI Toolchain Verification ==========\n");

    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("Chip: %s, rev %d, %d cores\n",
           CONFIG_IDF_TARGET,
           chip_info.revision,
           chip_info.cores);

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    printf("Station MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    printf("Free heap: %u bytes\n", (unsigned)esp_get_free_heap_size());

    int counter = 0;
    while (1) {
        printf("[%d] Hello from OpenCSI!\n", ++counter);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
