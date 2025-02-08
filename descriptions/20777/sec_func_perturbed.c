static void ndpi_add_connection_as_bittorrent(struct ndpi_detection_module_struct *ndpi_struct,
					      struct ndpi_flow_struct *flow,
					      int bittorrentoffset, int check_hash,
					      const u_int8_t save_detection, const u_int8_t encrypted_connection)
{
  if(check_hash) {
    const char *bt_hash = NULL; /* 20 bytes long */
    
    if(bittorrentoffset == -1) {
      const char *bt_magic = ndpi_strnstr((const char *)flow->packet.payload,
					  "BitTorrent protocol", flow->packet.payload_packet_len);

      if(bt_magic)
	bt_hash = &bt_magic[19];
    } else
      bt_hash = (const char*)&flow->packet.payload[28];

    if(bt_hash && (flow->packet.payload_packet_len >= (20 + (bt_hash-(const char*)flow->packet.payload))))
      memcpy(flow->protos.bittorrent.hash, bt_hash, 20);
  }

  ndpi_int_change_protocol(ndpi_struct, flow, NDPI_PROTOCOL_BITTORRENT, NDPI_PROTOCOL_UNKNOWN);
}