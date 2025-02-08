static void ndpi_add_connection_as_bittorrent(struct ndpi_detection_module_struct *ndpi_struct,
					      struct ndpi_flow_struct *flow,
					      int bt_offset, int check_hash,
					      const u_int8_t save_detection, const u_int8_t encrypted_connection)
{
  if(check_hash) {
    // <MASK>
  }

  ndpi_int_change_protocol(ndpi_struct, flow, NDPI_PROTOCOL_BITTORRENT, NDPI_PROTOCOL_UNKNOWN);
}