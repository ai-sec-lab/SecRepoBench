memcpy(flow->protos.tftp.filename, &packet->payload[2], ndpi_min(filename_len, sizeof(flow->protos.tftp.filename) - 1));
            flow->protos.tftp.filename[filename_len] = '\0';