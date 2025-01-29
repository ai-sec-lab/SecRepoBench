/*
 * h323.c
 *
 * Copyright (C) 2015-20 ntop.org
 * Copyright (C) 2013 Remy Mudingay <mudingay@ill.fr>
 *
 */

#include "ndpi_protocol_ids.h"

#define NDPI_CURRENT_PROTO NDPI_PROTOCOL_H323

#include "ndpi_api.h"


struct tpkt {
  u_int8_t version, reserved;
  u_int16_t len;
};

void ndpi_search_h323(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;
  u_int16_t dport = 0, sport = 0;

  NDPI_LOG_DBG(ndpi_struct, "search H323\n");

  /*
    The TPKT protocol is used by ISO 8072 (on port 102)
    and H.323. So this check below is to avoid ambiguities
  */
  if((packet->tcp != NULL) && (packet->tcp->dest != ntohs(102))) // <MASK>
}

void init_h323_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id, NDPI_PROTOCOL_BITMASK *detection_bitmask)
{
  ndpi_set_bitmask_protocol_detection("H323", ndpi_struct, detection_bitmask, *id,
				      NDPI_PROTOCOL_H323,
				      ndpi_search_h323,
				      NDPI_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD,
				      SAVE_DETECTION_BITMASK_AS_UNKNOWN,
				      ADD_TO_DETECTION_BITMASK);

  *id += 1;
}
