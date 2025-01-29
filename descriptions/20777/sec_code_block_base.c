const char *bt_hash = NULL; /* 20 bytes long */
    
    if(bt_offset == -1) {
      const char *bt_magic = ndpi_strnstr((const char *)flow->packet.payload,
					  "BitTorrent protocol", flow->packet.payload_packet_len);

      if(bt_magic)
	bt_hash = &bt_magic[19];
    } else
      bt_hash = (const char*)&flow->packet.payload[28];

    if(bt_hash && (flow->packet.payload_packet_len >= (20 + (bt_hash-(const char*)flow->packet.payload))))
      memcpy(flow->protos.bittorrent.hash, bt_hash, 20);