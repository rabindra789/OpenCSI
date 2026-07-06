#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <stdint.h>
#include "esp_wifi.h"

void transport_write_csi_record(wifi_csi_info_t *data, uint32_t total_count);

#endif
