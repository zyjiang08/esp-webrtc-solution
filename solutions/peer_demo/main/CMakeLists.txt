
idf_component_register(SRCS "webrtc.c"  "main.c" "board.c" "media_sys.c"
                       INCLUDE_DIRS "."
                       REQUIRES common console media_lib_sal libpeer esp_peer esp_webrtc esp_timer esp_system
                                av_render codec_board espressif__esp_capture
                                espressif__esp_audio_codec espressif__esp_codec_dev)
