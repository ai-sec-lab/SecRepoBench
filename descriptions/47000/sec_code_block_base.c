case 0x07: /* Open Connection Request 2 */
      ip_addr_offset = raknet_dissect_ip(packet, 17);
      if (ip_addr_offset == 0 ||
          !((ip_addr_offset == 16 && packet->payload_packet_len == 46) ||
            (ip_addr_offset == 4 && packet->payload_packet_len == 34)))
      {
        NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
        return;
      }

      {
          u_int16_t mtu_size = ntohs(get_u_int16_t(packet->payload, 20 + ip_addr_offset));
          if (mtu_size > 1500 /* Max. supported MTU, see: http://www.jenkinssoftware.com/raknet/manual/programmingtips.html */)
          {
            NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
            return;
          }
      }
      break;

    case 0x08: /* Open Connection Reply 2 */
      ip_addr_offset = raknet_dissect_ip(packet, 25);
      if (ip_addr_offset == 0 ||
          !((ip_addr_offset == 16 && packet->payload_packet_len == 47) ||
            (ip_addr_offset == 4 && packet->payload_packet_len == 35)))
      {
        NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
        return;
      }

      {
          u_int16_t mtu_size = ntohs(get_u_int16_t(packet->payload, 28 + ip_addr_offset));
          if (mtu_size > 1500 /* Max. supported MTU, see: http://www.jenkinssoftware.com/raknet/manual/programmingtips.html */)
          {
            NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
            return;
          }
      }
      break;

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