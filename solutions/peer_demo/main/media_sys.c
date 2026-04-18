/* Media system

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "codec_init.h"
#include "codec_board.h"
#include "esp_codec_dev.h"
#include "av_render.h"
#include "av_render_default.h"
#include "common.h"
#include "esp_audio_dec_default.h"
#include "esp_audio_enc_default.h"
#include "esp_capture.h"
#include "esp_capture_defaults.h"
#include "esp_capture_sink.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "media_lib_os.h"
#include "settings.h"

#define TAG "MEDIA_SYS"

#define AUDIO_SAMPLE_RATE     8000
#define AUDIO_CHANNEL         1
#define AUDIO_BITS_PER_SAMPLE 16

#define RET_ON_NULL(ptr, v) do {                                \
    if ((ptr) == NULL) {                                        \
        ESP_LOGE(TAG, "Memory allocate fail on %d", __LINE__); \
        return (v);                                             \
    }                                                           \
} while (0)

typedef struct {
    esp_capture_handle_t       capture_handle;
    esp_capture_audio_src_if_t *aud_src;
} capture_system_t;

typedef struct {
    audio_render_handle_t audio_render;
    av_render_handle_t    player;
} player_system_t;

static capture_system_t capture_sys;
static player_system_t  player_sys;

static int build_capture_system(void)
{
    esp_capture_audio_dev_src_cfg_t codec_cfg = {
        .record_handle = get_record_handle(),
    };
    capture_sys.aud_src = esp_capture_new_audio_dev_src(&codec_cfg);
    RET_ON_NULL(capture_sys.aud_src, -1);

    esp_capture_cfg_t cfg = {
        .sync_mode = ESP_CAPTURE_SYNC_MODE_AUDIO,
        .audio_src = capture_sys.aud_src,
    };
    if (esp_capture_open(&cfg, &capture_sys.capture_handle) != ESP_CAPTURE_ERR_OK) {
        ESP_LOGE(TAG, "Fail to open capture system");
        return -1;
    }
    return 0;
}

static int build_player_system(void)
{
    i2s_render_cfg_t i2s_cfg = {
        .fixed_clock = true,
        .play_handle = get_playback_handle(),
    };
    player_sys.audio_render = av_render_alloc_i2s_render(&i2s_cfg);
    RET_ON_NULL(player_sys.audio_render, -1);
    esp_codec_dev_set_out_vol(i2s_cfg.play_handle, DEFAULT_PLAYBACK_VOL);

    av_render_cfg_t render_cfg = {
        .audio_render = player_sys.audio_render,
        .audio_raw_fifo_size = 4096,
        .audio_render_fifo_size = 6 * 1024,
        .allow_drop_data = false,
    };
    player_sys.player = av_render_open(&render_cfg);
    if (player_sys.player == NULL) {
        ESP_LOGE(TAG, "Fail to create player");
        return -1;
    }
    return 0;
}

int media_sys_buildup(void)
{
    esp_audio_enc_register_default();
    esp_audio_dec_register_default();
    if (build_capture_system() != 0) {
        return -1;
    }
    if (build_player_system() != 0) {
        return -1;
    }
    return 0;
}

int media_sys_get_provider(esp_webrtc_media_provider_t *provider)
{
    if (provider == NULL) {
        return -1;
    }
    provider->capture = capture_sys.capture_handle;
    provider->player = player_sys.player;
    return 0;
}

int test_capture_to_player(void)
{
    esp_capture_sink_cfg_t sink_cfg = {
        .audio_info = {
            .format_id = ESP_CAPTURE_FMT_ID_G711A,
            .sample_rate = AUDIO_SAMPLE_RATE,
            .channel = AUDIO_CHANNEL,
            .bits_per_sample = AUDIO_BITS_PER_SAMPLE,
        },
    };
    esp_capture_sink_handle_t capture_path = NULL;
    if (esp_capture_sink_setup(capture_sys.capture_handle, 0, &sink_cfg, &capture_path) != ESP_CAPTURE_ERR_OK) {
        ESP_LOGE(TAG, "Fail to setup capture sink");
        return -1;
    }
    esp_capture_sink_enable(capture_path, ESP_CAPTURE_RUN_MODE_ALWAYS);

    av_render_audio_info_t render_aud_info = {
        .codec = AV_RENDER_AUDIO_CODEC_G711A,
        .sample_rate = AUDIO_SAMPLE_RATE,
        .channel = AUDIO_CHANNEL,
        .bits_per_sample = AUDIO_BITS_PER_SAMPLE,
    };
    av_render_add_audio_stream(player_sys.player, &render_aud_info);

    uint32_t start_time = (uint32_t)(esp_timer_get_time() / 1000);
    esp_capture_start(capture_sys.capture_handle);
    while ((uint32_t)(esp_timer_get_time() / 1000) < start_time + 2000) {
        media_lib_thread_sleep(30);
        esp_capture_stream_frame_t frame = {
            .stream_type = ESP_CAPTURE_STREAM_TYPE_AUDIO,
        };
        while (esp_capture_sink_acquire_frame(capture_path, &frame, true) == ESP_CAPTURE_ERR_OK) {
            av_render_audio_data_t audio_data = {
                .data = frame.data,
                .size = frame.size,
                .pts = frame.pts,
            };
            av_render_add_audio_data(player_sys.player, &audio_data);
            esp_capture_sink_release_frame(capture_path, &frame);
        }
    }
    esp_capture_stop(capture_sys.capture_handle);
    av_render_reset(player_sys.player);
    return 0;
}

int play_music(const uint8_t *data, int size, int duration)
{
    (void)data;
    (void)size;
    (void)duration;
    return 0;
}

int stop_music(void)
{
    return 0;
}
