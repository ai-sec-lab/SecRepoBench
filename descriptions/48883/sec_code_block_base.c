filename_len = ndpi_min(filename_len, sizeof(flow->protos.tftp.filename) - 1);
            memcpy(flow->protos.tftp.filename, &packet->payload[2], filename_len);
            flow->protos.tftp.filename[filename_len] = '\0';