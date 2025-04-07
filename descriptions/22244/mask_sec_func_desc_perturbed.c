void ndpi_search_openvpn(struct ndpi_detection_module_struct* ndpi_struct,
                         struct ndpi_flow_struct* flow) {
  struct ndpi_packet_struct* packet = &flow->packet;
  const u_int8_t * ovpn_payload = packet->payload;
  const u_int8_t * session_remote;
  u_int8_t operationcode;
  u_int8_t alen;
  int8_t hmac_size;
  int8_t failed = 0;
  /* No u_ */
  // Analyze the packet payload to determine if it might be an OpenVPN packet.
  // If the payload length is sufficient, adjust the payload pointer if it's over TCP.
  // Extract the operation code from the payload and, if it's over UDP, perform additional checks:
  //   - If the first packet in the flow matches specific length and operation code criteria, identify it as OpenVPN.
  //   - Otherwise, check if the flow's OpenVPN counter is below a threshold and the operation code indicates a client reset.
  //     - If so, attempt to extract and store the session ID from the payload.
  //   - If the counter is within a range and the operation code indicates a server reset, verify the session ID matches.
  //     - If a match is found, identify the protocol as OpenVPN; otherwise, mark the attempt as failed.
  // Increment the flow's OpenVPN counter after processing.
  // If the detection attempt fails, exclude the protocol from the flow's possible protocols.
  // <MASK>
}