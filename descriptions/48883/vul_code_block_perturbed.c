memcpy(flow->protos.tftp.filename, &packet->payload[2], ndpi_min(namelength, sizeof(flow->protos.tftp.filename) - 1));
            flow->protos.tftp.filename[namelength] = '\0';