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