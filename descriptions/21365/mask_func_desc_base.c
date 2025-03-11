static void ndpi_int_netbios_add_connection(struct ndpi_detection_module_struct *ndpi_struct,
					    struct ndpi_flow_struct *flow,
					    u_int16_t sub_protocol) { 
  // Interpret the NetBIOS name from the packet payload and store it in the flow's host server name.
  // Determine the offset to start interpreting the name based on the payload's content.
  // Update the detected protocol for the flow based on the sub-protocol value.
  // <MASK>
}