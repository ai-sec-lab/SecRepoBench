void ndpi_search_tftp(struct ndpi_detection_module_struct
		      *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *tftp_packet = &ndpi_struct->packet;

  NDPI_LOG_DBG(ndpi_struct, "search TFTP\n");

  if (tftp_packet->payload_packet_len < 4 /* min. header size */ ||
      get_u_int8_t(tftp_packet->payload, 0) != 0x00)
  {
    NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
    return;
  }

  /* parse TFTP opcode */
  switch (get_u_int8_t(tftp_packet->payload, 1))
  {
    case 0x01:
        /* Read request (RRQ) */
    case 0x02:
        /* Write request (WWQ) */

        if (tftp_packet->payload[tftp_packet->payload_packet_len - 1] != 0x00 /* last pdu element is a nul terminated string */)
        {
          NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
          return;
        }

        {
          char const * const possible_modes[] = { "netascii", "octet", "mail" };
          uint8_t mode_found = 0, mode_idx;
          size_t mode_len;

          for(mode_idx = 0; mode_idx < NDPI_ARRAY_LENGTH(possible_modes); ++mode_idx)
          {
            mode_len = strlen(possible_modes[mode_idx]);

            if (tftp_packet->payload_packet_len < mode_len + 1 /* mode is a nul terminated string */)
            {
              continue;
            }
            if (strncasecmp((char const *)&tftp_packet->payload[tftp_packet->payload_packet_len - 1 - mode_len],
                            possible_modes[mode_idx], mode_len) == 0)
            {
              mode_found = 1;
              break;
            }
          }

          if (mode_found == 0)
          {
            NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
            return;
          }

          /* Dissect RRQ/WWQ filename. */
          size_t filename_len = tftp_packet->payload_packet_len - 2 /* Opcode */ - mode_len - 1 /* NUL */;

          if (filename_len == 0 || tftp_packet->payload[2] == '\0' || ndpi_is_printable_buffer(&tftp_packet->payload[2], filename_len - 1) == 0)
          {
            ndpi_set_risk(ndpi_struct, flow, NDPI_MALFORMED_PACKET, "Invalid TFTP RR/WR header: Source/Destination file missing");
          } else {
            filename_len = ndpi_min(filename_len, sizeof(flow->protos.tftp.filename) - 1);
            memcpy(flow->protos.tftp.filename, &tftp_packet->payload[2], filename_len);
            flow->protos.tftp.filename[filename_len] = '\0';
          }

          /* We have seen enough and do not need any more TFTP packets. */
          NDPI_LOG_INFO(ndpi_struct, "found tftp (RRQ/WWQ)\n");
          ndpi_int_tftp_add_connection(ndpi_struct, flow);
        }
        return;

    case 0x03:
        /* Data (DATA) */
        if (tftp_packet->payload_packet_len > 4 /* DATA header size */ + 512 /* max. block size */)
        {
          NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
          return;
        }
        break;

    case 0x04:
        /* Acknowledgment (ACK) */
        if (tftp_packet->payload_packet_len != 4 /* ACK has a fixed packet size */)
        {
          NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
          return;
        }
        break;

    case 0x05:
        /* Error (ERROR) */

        if (tftp_packet->payload_packet_len < 5 ||
            tftp_packet->payload[tftp_packet->payload_packet_len - 1] != 0x00 ||
            tftp_packet->payload[2] != 0x00 || tftp_packet->payload[3] > 0x07)
        {
          NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
          return;
        }
        break;

    default:
        NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
        return;
  }

  if (flow->l4.udp.tftp_stage < 3)
  {
    NDPI_LOG_DBG2(ndpi_struct, "maybe tftp. need next packet\n");
    flow->l4.udp.tftp_stage++;
    return;
  }

  NDPI_LOG_INFO(ndpi_struct, "found tftp\n");
  ndpi_int_tftp_add_connection(ndpi_struct, flow);
}