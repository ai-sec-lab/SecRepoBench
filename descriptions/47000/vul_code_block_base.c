case 0x07: /* Open Connection Request 2 */
      ip_addr_offset = raknet_dissect_ip(packet, 17);
      if (packet->payload_packet_len != 34 || ip_addr_offset == 0)
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
      if (packet->payload_packet_len != 35 || ip_addr_offset == 0)
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