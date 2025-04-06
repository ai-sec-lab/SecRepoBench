/*
 * openvpn.c
 *
 * Copyright (C) 2011-20 - ntop.org
 *
 * OpenVPN TCP / UDP Detection - 128/160 hmac
 *
 * Detection based upon these openvpn protocol properties:
 *   - opcode
 *   - packet ID
 *   - session ID
 *
 * Two (good) packets are needed to perform detection.
 *  - First packet from client: save session ID
 *  - Second packet from server: report saved session ID
 *
 * TODO
 *  - Support PSK only mode (instead of TLS)
 *  - Support PSK + TLS mode (PSK used for early authentication)
 *  - TLS certificate extraction
 *
 */

#include "ndpi_protocol_ids.h"

#define NDPI_CURRENT_PROTO NDPI_PROTOCOL_OPENVPN

#include "ndpi_api.h"


#define P_CONTROL_HARD_RESET_CLIENT_V1  (0x01 << 3)
#define P_CONTROL_HARD_RESET_CLIENT_V2  (0x07 << 3)
#define P_CONTROL_HARD_RESET_SERVER_V1  (0x02 << 3)
#define P_CONTROL_HARD_RESET_SERVER_V2  (0x08 << 3)
#define P_OPCODE_MASK 0xF8
#define P_SHA1_HMAC_SIZE 20
#define P_HMAC_128 16                            // (RSA-)MD5, (RSA-)MD4, ..others
#define P_HMAC_160 20                            // (RSA-|DSA-)SHA(1), ..others, SHA1 is openvpn default
#define P_HARD_RESET_PACKET_ID_OFFSET(hmac_size) (9 + hmac_size)
#define P_PACKET_ID_ARRAY_LEN_OFFSET(hmac_size)  (P_HARD_RESET_PACKET_ID_OFFSET(hmac_size) + 8)
#define P_HARD_RESET_CLIENT_MAX_COUNT  5

static 
#ifndef WIN32
inline 
#endif
u_int32_t get_packet_id(const u_int8_t * payload, u_int8_t hms) {
  return(ntohl(*(u_int32_t*)(payload + P_HARD_RESET_PACKET_ID_OFFSET(hms))));
}

static 
#ifndef WIN32
inline
#endif
int8_t check_pkid_and_detect_hmac_size(const u_int8_t * payload) {
  // try to guess
  if(get_packet_id(payload, P_HMAC_160) == 1)
    return P_HMAC_160;
  
  if(get_packet_id(payload, P_HMAC_128) == 1)    
    return P_HMAC_128;
  
  return(-1);
}

void ndpi_search_openvpn(struct ndpi_detection_module_struct* ndpi_struct,
                         struct ndpi_flow_struct* flow) {
  struct ndpi_packet_struct* packet = &flow->packet;
  const u_int8_t * ovpn_payload = packet->payload;
  const u_int8_t * session_remote;
  u_int8_t operationcode;
  u_int8_t alen;
  int8_t hmac_size;
  int8_t failed = 0;
  /* No u_ */
  // Analyze the packet payload to determine if it might be an OpenVPN packet.
  // If the payload length is sufficient, adjust the payload pointer if it's over TCP.
  // Extract the operation code from the payload and, if it's over UDP, perform additional checks:
  //   - If the first packet in the flow matches specific length and operation code criteria, identify it as OpenVPN.
  //   - Otherwise, check if the flow's OpenVPN counter is below a threshold and the operation code indicates a client reset.
  //     - If so, attempt to extract and store the session ID from the payload.
  //   - If the counter is within a range and the operation code indicates a server reset, verify the session ID matches.
  //     - If a match is found, identify the protocol as OpenVPN; otherwise, mark the attempt as failed.
  // Increment the flow's OpenVPN counter after processing.
  // If the detection attempt fails, exclude the protocol from the flow's possible protocols.
  // <MASK>
}

void init_openvpn_dissector(struct ndpi_detection_module_struct *ndpi_struct,
			    u_int32_t *id, NDPI_PROTOCOL_BITMASK *detection_bitmask) {
  ndpi_set_bitmask_protocol_detection("OpenVPN", ndpi_struct, detection_bitmask, *id,
				      NDPI_PROTOCOL_OPENVPN,
				      ndpi_search_openvpn,
				      NDPI_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD,
				      SAVE_DETECTION_BITMASK_AS_UNKNOWN,
				      ADD_TO_DETECTION_BITMASK);

  *id += 1;
}
