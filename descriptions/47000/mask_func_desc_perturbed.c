void ndpi_search_raknet(struct ndpi_detection_module_struct *ndpi_struct,
                        struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct * const packet = &ndpi_struct->packet;
  u_int8_t op, ip_addr_offset, required_packets = 3;

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
      required_packets = 6;
      break;

    case 0x01: /* Unconnected Ping */
    case 0x02: /* Unconnected Ping */
      if (packet->payload_packet_len != 32)
      {
        NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
        return;
      }
      required_packets = 6;
      break;

    case 0x03: /* Connected Pong */
      if (packet->payload_packet_len != 16)
      {
        NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
        return;
      }
      required_packets = 6;
      break;

    case 0x05: /* Open Connection Request 1 */
      if (packet->payload_packet_len < 18 ||
          packet->payload[17] > 10 /* maximum supported protocol version */)
      {
        NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
        return;
      }
      required_packets = 6;
      break;

    case 0x06: /* Open Connection Reply 1 */
      if (packet->payload_packet_len != 28 ||
          packet->payload[25] > 0x01 /* connection uses encryption: bool -> 0x00 or 0x01 */)
      {
        NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
        return;
      }

      {
        u_int16_t max_mtu_size = ntohs(get_u_int16_t(packet->payload, 26));
        if (max_mtu_size > 1500 /* Max. supported MTU, see: http://www.jenkinssoftware.com/raknet/manual/programmingtips.html */)
        {
          NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
          return;
        }
      }
      required_packets = 4;
      break;

    // Handle RakNet protocol packets for Open Connection Request 2 and Open Connection Reply 2.
    // Extract the IP address offset from the packet and verify the packet length.
    // Ensure the calculated MTU size does not exceed a predefined maximum limit.
    // Exclude the protocol from further detection if any validation fails.
    // <MASK>

    case 0x10: /* Connection Request Accepted */
    case 0x13: /* New Incoming Connection */
      {
        ip_addr_offset = 4 + raknet_dissect_ip(packet, 0);
        if (op == 0x10)
        {
          ip_addr_offset += 2; // System Index
        }
        for (size_t i = 0; i < 10; ++i)
        {
          ip_addr_offset += 3 + raknet_dissect_ip(packet, ip_addr_offset);
        }
        ip_addr_offset += 16;
        if (ip_addr_offset != packet->payload_packet_len)
        {
          NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
          return;
        }
      }
      break;

    /* Check for Frame Set Packet's */
    case 0x80:
    case 0x81:
    case 0x82:
    case 0x83:
    case 0x84:
    case 0x85:
    case 0x86:
    case 0x87:
    case 0x88:
    case 0x89:
    case 0x8a:
    case 0x8b:
    case 0x8c:
    case 0x8d:
      {
        size_t frame_offset = 4;

        do {
          u_int8_t msg_flags = get_u_int8_t(packet->payload, frame_offset);
          if ((msg_flags & 0x0F) != 0)
          {
            NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
            return;
          }

          u_int16_t msg_size = ntohs(get_u_int16_t(packet->payload, frame_offset + 1));
          msg_size /= 8;
          if (msg_size == 0)
          {
            NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
            break;
          }

          u_int8_t reliability_type = (msg_flags & 0xE0) >> 5;
          if (reliability_type >= 2 && reliability_type <= 4 /* is reliable? */)
          {
            frame_offset += 3;
          }
          if (reliability_type == 1 || reliability_type == 4 /* is sequenced? */)
          {
            frame_offset += 3;
          }
          if (reliability_type == 3 || reliability_type == 7 /* is ordered? */)
          {
            frame_offset += 4;
          }
          if ((msg_flags & 0x10) != 0 /* is fragmented? */)
          {
            frame_offset += 10;
          }

          frame_offset += msg_size + 3;
        } while (frame_offset + 3 <= packet->payload_packet_len);

        /* We've dissected enough to be sure. */
        if (frame_offset == packet->payload_packet_len)
        {
          ndpi_int_raknet_add_connection(ndpi_struct, flow);
        } else {
          NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
        }
        return;
      }
      break;

    case 0x09: /* Connection Request */
      if (packet->payload_packet_len != 16)
      {
        NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
        return;
      }
      required_packets = 6;
      break;

    case 0x15: /* Disconnect */
      required_packets = 8;
      break;

    case 0x19: /* Incompatible Protocol */
      if (packet->payload_packet_len != 25 ||
          packet->payload[17] > 10)
      {
        NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
        return;
      }
      break;

    case 0x1c: /* Unconnected Pong */
      if (packet->payload_packet_len < 35)
      {
        NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
        return;
      }

      {
        u_int16_t motd_len = ntohs(get_u_int16_t(packet->payload, 33));

        if (motd_len == 0 || motd_len + 35 != packet->payload_packet_len)
        {
          NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
          return;
        }
      }
      break;

    case 0xa0: /* NACK */
    case 0xc0: /* ACK */
      {
        u_int16_t record_count = ntohs(get_u_int16_t(packet->payload, 1));
        size_t record_index = 0, record_offset = 3;

        do {
          if (packet->payload[record_offset] == 0x00 /* Range */)
          {
            record_offset += 7;
          } else if (packet->payload[record_offset] == 0x01 /* No Range */)
          {
            record_offset += 4;
          } else {
            NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
            return;
          }
        } while (++record_index < record_count &&
                 record_offset + 4 <= packet->payload_packet_len);

        if (record_index == record_count && record_offset == packet->payload_packet_len)
        {
          ndpi_int_raknet_add_connection(ndpi_struct, flow);
        } else {
          NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
        }
        return;
      }
      break;

    case 0xfe: /* Game Packet */
      required_packets = 8;
      break;

    default: /* Invalid RakNet packet */
      NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
      return;
  }

  if (flow->packet_counter < required_packets)
  {
    return;
  }

  ndpi_int_raknet_add_connection(ndpi_struct, flow);
}