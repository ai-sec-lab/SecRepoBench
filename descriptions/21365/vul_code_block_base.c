ndpi_int_netbios_add_connection(struct ndpi_detection_module_struct *ndpi_struct,
					    struct ndpi_flow_struct *flow,
					    u_int16_t sub_protocol) { 
  char name[64];
  u_int off = flow->packet.payload[12] == 0x20 ? 12 : 14;
  
  if(ndpi_netbios_name_interpret((char*)&flow->packet.payload[off], flow->packet.payload_packet_len - off, name, sizeof(name)) > 0)
    snprintf((char*)flow->host_server_name, sizeof(flow->host_server_name)-1, "%s", name);    

  if(sub_protocol == NDPI_PROTOCOL_UNKNOWN)
    ndpi_set_detected_protocol(ndpi_struct, flow, NDPI_PROTOCOL_NETBIOS, NDPI_PROTOCOL_UNKNOWN);
  else
    ndpi_set_detected_protocol(ndpi_struct, flow, sub_protocol, NDPI_PROTOCOL_NETBIOS);
}