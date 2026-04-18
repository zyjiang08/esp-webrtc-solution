#ifndef RTCP_H_
#define RTCP_H_

#ifdef __BYTE_ORDER
#define __BIG_ENDIAN 4321
#define __LITTLE_ENDIAN 1234
#elif __APPLE__
#include <machine/endian.h>
#else
#include <endian.h>
#endif

typedef enum RtcpType {

  RTCP_FIR = 192,
  RTCP_SR = 200,
  RTCP_RR = 201,
  RTCP_SDES = 202,
  RTCP_BYE = 203,
  RTCP_APP = 204,
  RTCP_RTPFB = 205,
  RTCP_PSFB = 206,
  RTCP_XR = 207,

} RtcpType;

typedef struct RtcpHeader {
  uint8_t flags;
  uint8_t type;
  uint16_t length;

} RtcpHeader;

typedef struct RtcpReportBlock {
  uint32_t ssrc;
  uint32_t flcnpl;
  uint32_t ehsnr;
  uint32_t jitter;
  uint32_t lsr;
  uint32_t dlsr;

} RtcpReportBlock;

typedef struct RtcpRr {
  RtcpHeader header;
  uint32_t ssrc;
  RtcpReportBlock report_block[1];

} RtcpRr;

typedef struct RtcpFir {
  uint32_t ssrc;
  uint32_t seqnr;

} RtcpFir;

typedef struct RtcpFb {
  RtcpHeader header;
  uint32_t ssrc;
  uint32_t media;
  char fci[1];

} RtcpFb;

static inline void rtcp_header_set_version(RtcpHeader* header, uint8_t version) {
  header->flags = (header->flags & 0x3F) | ((version & 0x03) << 6);
}

static inline void rtcp_header_set_padding(RtcpHeader* header, uint8_t padding) {
  header->flags = (header->flags & 0xDF) | ((padding & 0x01) << 5);
}

static inline void rtcp_header_set_rc(RtcpHeader* header, uint8_t rc) {
  header->flags = (header->flags & 0xE0) | (rc & 0x1F);
}

static inline uint8_t rtcp_header_get_version(const RtcpHeader* header) {
  return header->flags >> 6;
}

static inline uint8_t rtcp_header_get_rc(const RtcpHeader* header) {
  return header->flags & 0x1F;
}

int rtcp_probe(uint8_t* packet, size_t size);

int rtcp_get_pli(uint8_t* packet, int len, uint32_t ssrc);

int rtcp_get_fir(uint8_t* packet, int len, int* seqnr);

RtcpRr rtcp_parse_rr(uint8_t* packet);

#endif  // RTCP_H_
