/* Dissect RRQ/WWQ filename. */
          size_t filename_len = packet->payload_packet_len - 2 /* Opcode */ - mode_len - 1 /* NUL */;

          if (filename_len == 0 || packet->payload[2] == '\0' || ndpi_is_printable_buffer(&packet->payload[2], filename_len - 1) == 0)
          {
            ndpi_set_risk(ndpi_struct, flow, NDPI_MALFORMED_PACKET, "Invalid TFTP RR/WR header: Source/Destination file missing");
          } else {
            memcpy(flow->protos.tftp.filename, &packet->payload[2], ndpi_min(filename_len, sizeof(flow->protos.tftp.filename) - 1));
            flow->protos.tftp.filename[filename_len] = '\0';
          }

          /* We have seen enough and do not need any more TFTP packets. */
          NDPI_LOG_INFO(ndpi_struct, "found tftp (RRQ/WWQ)\n");
          ndpi_int_tftp_add_connection(ndpi_struct, flow);