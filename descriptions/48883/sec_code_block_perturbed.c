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