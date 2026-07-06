#ifndef CSI_H
#define CSI_H

#include "esp_wifi.h"

typedef void (*csi_callback_t)(void *ctx, wifi_csi_info_t *data);

void csi_init(csi_callback_t callback);

#endif
