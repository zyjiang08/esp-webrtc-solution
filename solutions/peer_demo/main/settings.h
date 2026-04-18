/* General settings

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#pragma once

#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Set used board name, see `codec_board` README.md for more details
 */
#if CONFIG_IDF_TARGET_ESP32P4
#define TEST_BOARD_NAME "ESP32_P4_DEV_V14"
#else
#define TEST_BOARD_NAME "S3_Korvo_V2"
#endif

/**
 * @brief  Set for wifi ssid
 */
#define WIFI_SSID     "TP-LINK_harry"

/**
 * @brief  Set for wifi password
 */
#define WIFI_PASSWORD "edc123456"

/**
 * @brief  Set default playback volume
 */
#define DEFAULT_PLAYBACK_VOL (85)

#ifdef __cplusplus
}
#endif
