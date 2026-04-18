#include <stdio.h>
#include "esp_log.h"
#include "codec_init.h"
#include "codec_board.h"
#include "sdkconfig.h"
#include "settings.h"

static const char *TAG = "Board";

void init_board(void)
{
    ESP_LOGI(TAG, "Init board.");
    set_codec_board_type(TEST_BOARD_NAME);
    codec_init_cfg_t cfg = {
        .reuse_dev = false,
    };
    init_codec(&cfg);
}
