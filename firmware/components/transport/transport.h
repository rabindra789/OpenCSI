#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <stdint.h>
#include "esp_wifi.h"

void transport_begin(const char *exp_id, const char *scenario, int duration_s);
void transport_write_csi_record(wifi_csi_info_t *data, uint32_t seq_num, int64_t timestamp_us);
void transport_end(void);

#endif
