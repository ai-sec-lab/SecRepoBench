namelength = ndpi_min(namelength, sizeof(flow->protos.tftp.filename) - 1);
            memcpy(flow->protos.tftp.filename, &packet->payload[2], namelength);
            flow->protos.tftp.filename[namelength] = '\0';