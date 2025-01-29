/*
 * raknet.c
 *
 * Copyright (C) 2011-22 - ntop.org
 *
 * nDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nDPI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ndpi_protocol_ids.h"

#define NDPI_CURRENT_PROTO NDPI_PROTOCOL_RAKNET

#include "ndpi_api.h"

static void ndpi_int_raknet_add_connection(struct ndpi_detection_module_struct * const ndpi_struct,
                                           struct ndpi_flow_struct * const flow)
{
  NDPI_LOG_INFO(ndpi_struct, "found RakNet\n");
  ndpi_set_detected_protocol(ndpi_struct, flow,
                             NDPI_PROTOCOL_RAKNET,
                             NDPI_PROTOCOL_UNKNOWN,
                             NDPI_CONFIDENCE_DPI);
}

static size_t raknet_dissect_ip(struct ndpi_packet_struct * const packet, size_t offset)
{
  if (offset + 1 >= packet->payload_packet_len ||
      (packet->payload[offset] != 0x04 /* IPv4 */ &&
       packet->payload[offset] != 0x06 /* IPv6 */))
  {
    return 0;
  }

  return (packet->payload[offset] == 0x04 ? 4 : 16);
}

/* Reference: https://wiki.vg/Raknet_Protocol */
void ndpi_search_raknet(struct ndpi_detection_module_struct *ndpi_struct,
                        struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct * const packet = &ndpi_struct->packet;
  u_int8_t op, ip_addr_offset, neededpackets = 3;

  NDPI_LOG_DBG(ndpi_struct, "search RakNet\n");

  if (packet->udp == NULL || packet->payload_packet_len < 7)
  {
    NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
    return;
  }

  op = packet->payload[0];

  switch (op)
  {
    case 0x00: /* Connected Ping */
      if (packet->payload_packet_len != 8)
      {
        NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
        return;
      }
      neededpackets = 6;
      break;

    case 0x01: /* Unconnected Ping */
    case 0x02: /* Unconnected Ping */
      if (packet->payload_packet_len != 32)
      {
        NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
        return;
      }
      neededpackets = 6;
      break;

    case 0x03: /* Connected Pong */
      if (packet->payload_packet_len != 16)
      {
        NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
        return;
      }
      neededpackets = 6;
      break;

    case 0x05: /* Open Connection Request 1 */
      if (packet->payload_packet_len < 18 ||
          packet->payload[17] > 10 /* maximum supported protocol version */)
      {
        NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
        return;
      }
      neededpackets = 6;
      break;

    case 0x06: /* Open Connection Reply 1 */
      if (packet->payload_packet_len != 28 ||
          packet->payload[25] > 0x01 /* connection uses encryption: bool -> 0x00 or 0x01 */)
      {
        NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
        return;
      }

      {
        u_int16_t mtu_size = ntohs(get_u_int16_t(packet->payload, 26));
        if (mtu_size > 1500 /* Max. supported MTU, see: http://www.jenkinssoftware.com/raknet/manual/programmingtips.html */)
        {
          NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
          return;
        }
      }
      neededpackets = 4;
      break;

    // <MASK>
  }

  if (flow->packet_counter < neededpackets)
  {
    return;
  }

  ndpi_int_raknet_add_connection(ndpi_struct, flow);
}

void init_raknet_dissector(struct ndpi_detection_module_struct *ndpi_struct,
                           u_int32_t *id, NDPI_PROTOCOL_BITMASK *detection_bitmask)
{
  ndpi_set_bitmask_protocol_detection("RakNet", ndpi_struct, detection_bitmask, *id,
				      NDPI_PROTOCOL_RAKNET,
				      ndpi_search_raknet,
				      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_UDP_WITH_PAYLOAD,
				      SAVE_DETECTION_BITMASK_AS_UNKNOWN,
				      ADD_TO_DETECTION_BITMASK);

  *id += 1;
}
