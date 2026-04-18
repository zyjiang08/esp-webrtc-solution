/* OpenAI realtime communication Demo code

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "av_render.h"
#include "esp_random.h"
#include "esp_capture.h"
#include "esp_capture_sink.h"
#include "esp_log.h"
#include "esp_peer_signaling.h"
#include "esp_timer.h"
#include "media_lib_os.h"
#include "peer.h"
#include "peer_connection.h"
#include "common.h"

const esp_peer_signaling_impl_t *esp_signaling_get_apprtc_impl(void);

#define TAG "PEER_DEMO"

#define AUDIO_SAMPLE_RATE         8000
#define AUDIO_CHANNEL             1
#define AUDIO_BITS_PER_SAMPLE     16
#define AUDIO_FRAME_INTERVAL_MS   20
#define AUDIO_SEND_FRAME_MS       20
#define AUDIO_SEND_FRAME_BYTES    (AUDIO_SAMPLE_RATE * AUDIO_SEND_FRAME_MS / 1000)
#define AUDIO_SEND_BUFFER_BYTES   (AUDIO_SEND_FRAME_BYTES * 4)
#define TEST_PERIOD 1000

static esp_peer_signaling_handle_t signaling = NULL;
static PeerConnection *peer = NULL;
static esp_timer_handle_t timer;
static bool peer_running = false;
static bool peer_task_running = false;
static bool peer_lib_inited = false;
static bool peer_is_initiator = false;
static bool data_channel_ready = false;
static bool media_ready = false;
static bool media_stream_started = false;
static bool media_send_running = false;
static bool media_send_task_running = false;
static bool player_stream_ready = false;
static uint32_t recv_audio_pts = 0;
static uint8_t send_audio_buf[AUDIO_SEND_BUFFER_BYTES];
static size_t send_audio_buf_len = 0;
static uint32_t sent_audio_packets = 0;
static uint32_t sent_audio_active_packets = 0;
static uint32_t sent_audio_silence_packets = 0;
static int64_t sent_audio_log_time = 0;
static esp_capture_handle_t capture_handle = NULL;
static esp_capture_sink_handle_t capture_path = NULL;
static av_render_handle_t player_handle = NULL;
static SemaphoreHandle_t webrtc_lock = NULL;

#define DEFAULT_DATA_CHANNEL_LABEL "chat"

typedef struct {
    char    *sdp;
    SdpType  type;
    bool     create_answer;
} remote_sdp_task_t;

typedef struct {
    const char* question;
    const char* answer;
} data_channel_chat_content_t;

static bool webrtc_lock_take(TickType_t wait_ticks)
{
    if (webrtc_lock == NULL) {
        webrtc_lock = xSemaphoreCreateRecursiveMutex();
        if (webrtc_lock == NULL) {
            ESP_LOGE(TAG, "Fail to create webrtc lock");
            return false;
        }
    }
    return xSemaphoreTakeRecursive(webrtc_lock, wait_ticks) == pdTRUE;
}

static void webrtc_lock_give(void)
{
    if (webrtc_lock) {
        xSemaphoreGiveRecursive(webrtc_lock);
    }
}

static bool g711a_packet_looks_silence(const uint8_t *data, size_t size)
{
    if (data == NULL || size == 0) {
        return true;
    }
    size_t silence_like = 0;
    for (size_t i = 0; i < size; i++) {
        if (data[i] == 0xD5 || data[i] == 0x55) {
            silence_like++;
        }
    }
    return silence_like * 10 >= size * 9;
}

static data_channel_chat_content_t chat_content[] = {
    {"Hi!", "Hello!"},
    {"How are you?", "I am fine."},
    {"Wish to be your friend.", "Great!"},
    {"What's your name?", "I am a chatbot, nice to meet you!"},
    {"What do you do?", "I am here to chat and assist you with various tasks."},
    {"How old are you?", "I don't have an age. I was created to chat with you!"},
    {"Do you have hobbies?", "I enjoy chatting with you and learning new things."},
    {"Tell me a story.", "Once upon a time, a curious cat discovered a magical world..."},
    {"Tell me a joke.", "Why don't skeletons fight each other? They don't have the guts!"},
    {"What is the weather like?", "I am not sure, but you can check your local forecast."},
    {"What is your favorite color?", "I don't have a favorite color, but I like all of them!"},
    {"What is the time?", "Sorry, I can't tell the time. You can check your device for that."},
    {"What can you do?", "I can answer questions, tell jokes, help with tasks, and much more!"},
    {"Where are you from?", "I was created by developers, so I don't have a specific location."},
    {"What is love?", "Love is a complex emotion that connects people. What do you think love is?"},
    {"Do you like music?", "I don't listen to music, but I know about it! What's your favorite genre?"},
    {"Goodbye!", "Bye!"},
};

static int setup_media_pipeline(void)
{
    if (media_ready) {
        return 0;
    }
    esp_webrtc_media_provider_t media_provider = { 0 };
    if (media_sys_get_provider(&media_provider) != 0 || media_provider.capture == NULL || media_provider.player == NULL) {
        ESP_LOGE(TAG, "Media system is not ready");
        return -1;
    }
    capture_handle = media_provider.capture;
    player_handle = media_provider.player;

    esp_capture_sink_cfg_t sink_cfg = {
        .audio_info = {
            .format_id = ESP_CAPTURE_FMT_ID_G711A,
            .sample_rate = AUDIO_SAMPLE_RATE,
            .channel = AUDIO_CHANNEL,
            .bits_per_sample = AUDIO_BITS_PER_SAMPLE,
        },
    };
    esp_capture_err_t ret = esp_capture_sink_setup(capture_handle, 0, &sink_cfg, &capture_path);
    if (ret != ESP_CAPTURE_ERR_OK) {
        ESP_LOGE(TAG, "Fail to setup capture path ret:%d", ret);
        capture_path = NULL;
        return -1;
    }
    ret = esp_capture_sink_enable(capture_path, ESP_CAPTURE_RUN_MODE_ALWAYS);
    if (ret != ESP_CAPTURE_ERR_OK) {
        ESP_LOGE(TAG, "Fail to enable capture path ret:%d", ret);
        return -1;
    }
    media_ready = true;
    return 0;
}

static void media_send_task(void *arg)
{
    (void)arg;
    media_send_task_running = true;
    sent_audio_log_time = esp_timer_get_time();
    while (media_send_running) {
        if (peer && capture_path && peer_connection_get_state(peer) == PEER_CONNECTION_COMPLETED) {
            esp_capture_stream_frame_t audio_frame = {
                .stream_type = ESP_CAPTURE_STREAM_TYPE_AUDIO,
            };
            while (esp_capture_sink_acquire_frame(capture_path, &audio_frame, true) == ESP_CAPTURE_ERR_OK) {
                size_t offset = 0;
                while (offset < audio_frame.size) {
                    size_t copy_size = audio_frame.size - offset;
                    size_t free_size = AUDIO_SEND_BUFFER_BYTES - send_audio_buf_len;
                    if (copy_size > free_size) {
                        copy_size = free_size;
                    }
                    memcpy(send_audio_buf + send_audio_buf_len, audio_frame.data + offset, copy_size);
                    send_audio_buf_len += copy_size;
                    offset += copy_size;
                    while (send_audio_buf_len >= AUDIO_SEND_FRAME_BYTES) {
                        bool looks_silence = g711a_packet_looks_silence(send_audio_buf, AUDIO_SEND_FRAME_BYTES);
                        int ret = peer_connection_send_audio(peer, send_audio_buf, AUDIO_SEND_FRAME_BYTES);
                        if (ret != 0) {
                            ESP_LOGW(TAG, "Send audio frame failed: %d", ret);
                            break;
                        }
                        sent_audio_packets++;
                        if (looks_silence) {
                            sent_audio_silence_packets++;
                        } else {
                            sent_audio_active_packets++;
                        }
                        if (send_audio_buf_len > AUDIO_SEND_FRAME_BYTES) {
                            memmove(send_audio_buf,
                                    send_audio_buf + AUDIO_SEND_FRAME_BYTES,
                                    send_audio_buf_len - AUDIO_SEND_FRAME_BYTES);
                        }
                        send_audio_buf_len -= AUDIO_SEND_FRAME_BYTES;
                    }
                }
                esp_capture_sink_release_frame(capture_path, &audio_frame);
            }
            int64_t now = esp_timer_get_time();
            if (now - sent_audio_log_time >= 1000000) {
                ESP_LOGI(TAG, "Sent audio packets=%u active=%u silence_like=%u pending=%u",
                         (unsigned)sent_audio_packets,
                         (unsigned)sent_audio_active_packets,
                         (unsigned)sent_audio_silence_packets,
                         (unsigned)send_audio_buf_len);
                sent_audio_packets = 0;
                sent_audio_active_packets = 0;
                sent_audio_silence_packets = 0;
                sent_audio_log_time = now;
            }
        }
        media_lib_thread_sleep(AUDIO_FRAME_INTERVAL_MS);
    }
    media_send_task_running = false;
    media_lib_thread_destroy(NULL);
}

static int start_media_stream(void)
{
    if (media_stream_started) {
        return 0;
    }
    if (setup_media_pipeline() != 0) {
        return -1;
    }
    av_render_audio_info_t render_aud_info = {
        .codec = AV_RENDER_AUDIO_CODEC_G711A,
        .sample_rate = AUDIO_SAMPLE_RATE,
        .channel = AUDIO_CHANNEL,
        .bits_per_sample = AUDIO_BITS_PER_SAMPLE,
    };
    if (!player_stream_ready) {
        av_render_add_audio_stream(player_handle, &render_aud_info);
        player_stream_ready = true;
    }
    esp_capture_sink_enable(capture_path, ESP_CAPTURE_RUN_MODE_ALWAYS);
    esp_capture_err_t ret = esp_capture_start(capture_handle);
    if (ret != ESP_CAPTURE_ERR_OK) {
        ESP_LOGE(TAG, "Fail to start capture ret:%d", ret);
        return -1;
    }
    media_send_running = true;
    if (media_lib_thread_create_from_scheduler(NULL, "pc_send", media_send_task, NULL) != 0) {
        media_send_running = false;
        esp_capture_stop(capture_handle);
        ESP_LOGE(TAG, "Fail to create media send task");
        return -1;
    }
    media_stream_started = true;
    recv_audio_pts = 0;
    ESP_LOGI(TAG, "Real audio stream started");
    return 0;
}

static void stop_media_stream(void)
{
    if (media_send_running) {
        media_send_running = false;
        int retry = 100;
        while (media_send_task_running && retry-- > 0) {
            media_lib_thread_sleep(20);
        }
    }
    if (media_stream_started && capture_handle) {
        esp_capture_stop(capture_handle);
    }
    if (capture_path) {
        esp_capture_sink_enable(capture_path, ESP_CAPTURE_RUN_MODE_DISABLE);
    }
    if (player_handle) {
        av_render_reset(player_handle);
    }
    media_stream_started = false;
    player_stream_ready = false;
    recv_audio_pts = 0;
    send_audio_buf_len = 0;
    sent_audio_packets = 0;
    sent_audio_active_packets = 0;
    sent_audio_silence_packets = 0;
}

static void send_cb(void *ctx)
{
    if (peer && peer_connection_get_state(peer) == PEER_CONNECTION_COMPLETED) {
        if (data_channel_ready) {
            int question = esp_random() % (sizeof(chat_content) / sizeof(chat_content[0]));

            ESP_LOGI(TAG, "Send question:%s", chat_content[question].question);
            peer_connection_datachannel_send(peer,
                                             (char *)chat_content[question].question,
                                             strlen(chat_content[question].question) + 1);
        }
    }
}

static void peer_state_handler(PeerConnectionState state, void *ctx)
{
    ESP_LOGI(TAG, "Peer state: %s", peer_connection_state_to_string(state));
    if (state == PEER_CONNECTION_COMPLETED) {
        start_media_stream();
        if (peer_is_initiator && data_channel_ready == false) {
            int ret = peer_connection_create_datachannel(peer,
                                                         DATA_CHANNEL_RELIABLE,
                                                         0,
                                                         0,
                                                         DEFAULT_DATA_CHANNEL_LABEL,
                                                         "");
            if (ret != 0) {
                ESP_LOGW(TAG, "Create data channel failed: %d", ret);
            }
        }
        if (timer == NULL) {
            esp_timer_create_args_t cfg = { 
                .callback = send_cb,
                .name = "send",
            };
            esp_timer_create(&cfg, &timer);
            if (timer) {
                esp_timer_start_periodic(timer, TEST_PERIOD * 1000);
            }
        }
    } else if (state == PEER_CONNECTION_FAILED ||
               state == PEER_CONNECTION_DISCONNECTED ||
               state == PEER_CONNECTION_CLOSED) {
        stop_media_stream();
        data_channel_ready = false;
        if (timer) {
            esp_timer_stop(timer);
            esp_timer_delete(timer);
            timer = NULL;
        }
    }
}

static void peer_msg_handler(char *sdp, void *ctx)
{
    if (signaling && sdp) {
        esp_peer_signaling_msg_t msg = {
            .type = ESP_PEER_SIGNALING_MSG_SDP,
            .data = (uint8_t *)sdp,
            .size = strlen(sdp),
        };
        esp_peer_signaling_send_msg(signaling, &msg);
    }
}

static void peer_video_data_handler(uint8_t *data, size_t size, void *ctx)
{
    (void)data;
    (void)size;
    (void)ctx;
}

static void peer_audio_data_handler(uint8_t *data, size_t size, void *ctx)
{
    (void)ctx;
    if (player_handle == NULL || player_stream_ready == false || size == 0) {
        return;
    }
    av_render_audio_data_t audio_data = {
        .pts = recv_audio_pts,
        .data = data,
        .size = size,
    };
    av_render_add_audio_data(player_handle, &audio_data);
    recv_audio_pts += (uint32_t)((size * 1000ULL) / AUDIO_SAMPLE_RATE);
}

static void peer_data_open_handler(void *ctx)
{
    (void)ctx;
    data_channel_ready = true;
    ESP_LOGI(TAG, "Data channel opened");
}

static void peer_data_close_handler(void *ctx)
{
    (void)ctx;
    data_channel_ready = false;
    ESP_LOGI(TAG, "Data channel closed");
}

static void peer_data_handler(char *msg, size_t len, void *ctx, uint16_t sid)
{
    (void)ctx;
    (void)sid;
    if (msg == NULL || len == 0) {
        return;
    }
    int ans = -1;
    for (int i = 0; i < sizeof(chat_content) / sizeof(chat_content[0]); i++) {
        if (strcmp(msg, chat_content[i].question) == 0) {
            ans = i;
            break;
        }
    }
    if (ans >= 0) {
        ESP_LOGI(TAG, "Get question:%s", msg);
        ESP_LOGI(TAG, "Send answer:%s", chat_content[ans].answer);
        peer_connection_datachannel_send(peer,
                                         (char *)chat_content[ans].answer,
                                         strlen(chat_content[ans].answer) + 1);
    } else {
        ESP_LOGI(TAG, "Get answer:%s", msg);
    }
}

static void pc_task(void *arg)
{
    peer_task_running = true;
    while (peer_running) {
        peer_connection_loop(peer);
        media_lib_thread_sleep(20);
    }
    peer_task_running = false;
    media_lib_thread_destroy(NULL);
}

static void create_offer_task(void *arg)
{
    (void)arg;
    if (peer) {
        peer_connection_create_offer(peer);
    }
    media_lib_thread_destroy(NULL);
}

static void set_remote_sdp_task(void *arg)
{
    remote_sdp_task_t *task = (remote_sdp_task_t *)arg;
    if (peer && task && task->sdp) {
        peer_connection_set_remote_description(peer, task->sdp, task->type);
        if (task->create_answer) {
            peer_connection_create_answer(peer);
        }
    }
    if (task) {
        free(task->sdp);
        free(task);
    }
    media_lib_thread_destroy(NULL);
}

static int async_create_offer(void)
{
    return media_lib_thread_create_from_scheduler(NULL, "pc_offer", create_offer_task, NULL);
}

static int async_set_remote_sdp(const char *sdp, SdpType type, bool create_answer)
{
    remote_sdp_task_t *task = calloc(1, sizeof(remote_sdp_task_t));
    if (task == NULL) {
        return -1;
    }
    task->sdp = strdup(sdp);
    if (task->sdp == NULL) {
        free(task);
        return -1;
    }
    task->type = type;
    task->create_answer = create_answer;
    int ret = media_lib_thread_create_from_scheduler(NULL, "pc_remote", set_remote_sdp_task, task);
    if (ret != 0) {
        free(task->sdp);
        free(task);
    }
    return ret;
}

static int create_peer(esp_peer_signaling_ice_info_t *info, void *ctx)
{
    if (peer) {
        return 0;
    }
    if (setup_media_pipeline() != 0) {
        return -1;
    }
    if (peer_lib_inited == false) {
        if (peer_init() != 0) {
            ESP_LOGE(TAG, "peer_init failed");
            return -1;
        }
        peer_lib_inited = true;
    }
    PeerConfiguration cfg = {
        .audio_codec = CODEC_PCMA,
        .datachannel = DATA_CHANNEL_STRING,
        .onaudiotrack = peer_audio_data_handler,
        .onvideotrack = peer_video_data_handler,
        .user_data = ctx,
    };
    if (info) {
        cfg.ice_servers[0].urls = info->server_info.stun_url;
        cfg.ice_servers[0].username = info->server_info.user;
        cfg.ice_servers[0].credential = info->server_info.psw;
        peer_is_initiator = info->is_initiator;
    }
    peer = peer_connection_create(&cfg);
    if (peer == NULL) {
        ESP_LOGE(TAG, "peer_connection_create failed");
        return -1;
    }
    data_channel_ready = false;
    peer_connection_onicecandidate(peer, peer_msg_handler);
    peer_connection_oniceconnectionstatechange(peer, peer_state_handler);
    peer_connection_ondatachannel(peer,
                                  peer_data_handler,
                                  peer_data_open_handler,
                                  peer_data_close_handler);
    peer_running = true;
    media_lib_thread_create_from_scheduler(NULL, "pc_task", pc_task, NULL);
    return 0;
}

static void destroy_peer(void)
{
    stop_media_stream();
    peer_running = false;
    int retry = 100;
    while (peer_task_running && retry-- > 0) {
        media_lib_thread_sleep(20);
    }
    if (timer) {
        esp_timer_stop(timer);
        esp_timer_delete(timer);
        timer = NULL;
    }
    if (peer) {
        peer_connection_destroy(peer);
        peer = NULL;
    }
    data_channel_ready = false;
}

static int signaling_ice_info_handler(esp_peer_signaling_ice_info_t* info, void* ctx)
{
    return create_peer(info, ctx);
}

static int signaling_connected_handler(void* ctx)
{
    if (peer && peer_is_initiator) {
        return async_create_offer();
    }
    return 0;
}

static int signaling_msg_handler(esp_peer_signaling_msg_t* msg, void* ctx)
{
    if (msg->type == ESP_PEER_SIGNALING_MSG_BYE) {
        destroy_peer();
    } else if (msg->type == ESP_PEER_SIGNALING_MSG_SDP) {
        if (peer) {
            if (peer_is_initiator) {
                return async_set_remote_sdp((char *)msg->data, SDP_TYPE_ANSWER, false);
            } else {
                return async_set_remote_sdp((char *)msg->data, SDP_TYPE_OFFER, true);
            }
        }
    } else if (msg->type == ESP_PEER_SIGNALING_MSG_CANDIDATE) {
        if (peer) {
            peer_connection_add_ice_candidate(peer, (char *)msg->data);
        }
    }
    return 0;
}

static int signaling_close_handler(void *ctx)
{
    return 0;
}

static int start_signaling(char* url)
{
    esp_peer_signaling_cfg_t cfg = {
        .signal_url = url,
        .on_ice_info = signaling_ice_info_handler,
        .on_connected = signaling_connected_handler,
        .on_msg = signaling_msg_handler,
        .on_close = signaling_close_handler,
    };
    // Use APPRTC signaling
    return esp_peer_signaling_start(&cfg, esp_signaling_get_apprtc_impl(), &signaling);
}

int start_webrtc(char *url)
{
    if (webrtc_lock_take(pdMS_TO_TICKS(3000)) == false) {
        return -1;
    }
    if (network_is_connected() == false) {
        ESP_LOGE(TAG, "Wifi not connected yet");
        webrtc_lock_give();
        return -1;
    }
    stop_webrtc();
    int ret = start_signaling(url);
    webrtc_lock_give();
    return ret;
}

void query_webrtc(void)
{
    if (peer) {
        ESP_LOGI(TAG, "Current peer state: %s", peer_connection_state_to_string(peer_connection_get_state(peer)));
    }
}

int stop_webrtc(void)
{
    if (webrtc_lock_take(pdMS_TO_TICKS(3000)) == false) {
        return -1;
    }
    destroy_peer();
    if (signaling) {
        esp_peer_signaling_stop(signaling);
        signaling = NULL;
    }
    webrtc_lock_give();
    return 0;
}
