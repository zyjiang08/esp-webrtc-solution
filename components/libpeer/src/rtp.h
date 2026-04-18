#ifndef RTP_H_
#define RTP_H_

#include <stdint.h>

#ifdef __BYTE_ORDER
#define __BIG_ENDIAN 4321
#define __LITTLE_ENDIAN 1234
#elif __APPLE__
#include <machine/endian.h>
#else
#include <endian.h>
#endif

#include "config.h"
#include "peer_connection.h"

typedef enum RtpPayloadType {

  PT_PCMU = 0,
  PT_PCMA = 8,
  PT_G722 = 9,
  PT_H264 = 96,
  PT_OPUS = 111

} RtpPayloadType;

typedef enum RtpSsrc {

  SSRC_H264 = 1,
  SSRC_PCMA = 4,
  SSRC_PCMU = 5,
  SSRC_OPUS = 6,

} RtpSsrc;

typedef struct RtpHeader {
  uint8_t flags;
  uint8_t payload_type;
  uint16_t seq_number;
  uint32_t timestamp;
  uint32_t ssrc;
  uint32_t csrc[0];

} RtpHeader;

typedef struct RtpPacket {
  RtpHeader header;
  uint8_t payload[0];

} RtpPacket;

typedef struct RtpMap {
  int pt_h264;
  int pt_opus;
  int pt_pcma;

} RtpMap;

typedef struct RtpEncoder RtpEncoder;
typedef struct RtpDecoder RtpDecoder;
typedef void (*RtpOnPacket)(uint8_t* packet, size_t bytes, void* user_data);

struct RtpDecoder {
  RtpPayloadType type;
  RtpOnPacket on_packet;
  int (*decode_func)(RtpDecoder* rtp_decoder, uint8_t* data, size_t size);
  void* user_data;
};

struct RtpEncoder {
  RtpPayloadType type;
  RtpOnPacket on_packet;
  int (*encode_func)(RtpEncoder* rtp_encoder, uint8_t* data, size_t size);
  void* user_data;
  uint16_t seq_number;
  uint32_t ssrc;
  uint32_t timestamp;
  uint32_t timestamp_increment;
  uint8_t buf[CONFIG_MTU + 128];
};

static inline void rtp_header_set_version(RtpHeader* header, uint8_t version) {
  header->flags = (header->flags & 0x3F) | ((version & 0x03) << 6);
}

static inline void rtp_header_set_padding(RtpHeader* header, uint8_t padding) {
  header->flags = (header->flags & 0xDF) | ((padding & 0x01) << 5);
}

static inline void rtp_header_set_extension(RtpHeader* header, uint8_t extension) {
  header->flags = (header->flags & 0xEF) | ((extension & 0x01) << 4);
}

static inline void rtp_header_set_csrc_count(RtpHeader* header, uint8_t count) {
  header->flags = (header->flags & 0xF0) | (count & 0x0F);
}

static inline void rtp_header_set_marker(RtpHeader* header, uint8_t marker) {
  header->payload_type = (header->payload_type & 0x7F) | ((marker & 0x01) << 7);
}

static inline void rtp_header_set_payload_type(RtpHeader* header, uint8_t payload_type) {
  header->payload_type = (header->payload_type & 0x80) | (payload_type & 0x7F);
}

static inline uint8_t rtp_header_get_version(const RtpHeader* header) {
  return header->flags >> 6;
}

static inline uint8_t rtp_header_get_marker(const RtpHeader* header) {
  return header->payload_type >> 7;
}

static inline uint8_t rtp_header_get_payload_type(const RtpHeader* header) {
  return header->payload_type & 0x7F;
}

int rtp_packet_validate(uint8_t* packet, size_t size);

void rtp_encoder_init(RtpEncoder* rtp_encoder, MediaCodec codec, RtpOnPacket on_packet, void* user_data);

int rtp_encoder_encode(RtpEncoder* rtp_encoder, const uint8_t* data, size_t size);

void rtp_decoder_init(RtpDecoder* rtp_decoder, MediaCodec codec, RtpOnPacket on_packet, void* user_data);

int rtp_decoder_decode(RtpDecoder* rtp_decoder, const uint8_t* data, size_t size);

uint32_t rtp_get_ssrc(uint8_t* packet);

#endif  // RTP_H_
