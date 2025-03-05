static void ndpi_check_http_server(struct ndpi_detection_module_struct *ndpi_struct,
				   struct ndpi_flow_struct *flow,
				   const char *server, u_int server_len) {
  if(server_len > 7) {
    u_int off;
  
    if(strncmp((const char *)server, "ntopng ", 7) == 0) {
      ndpi_set_detected_protocol(ndpi_struct, flow, NDPI_PROTOCOL_NTOP, NDPI_PROTOCOL_HTTP, NDPI_CONFIDENCE_DPI);
      NDPI_CLR_BIT(flow->risk, NDPI_KNOWN_PROTOCOL_ON_NON_STANDARD_PORT);
    } else if((strncasecmp(server, "Apache/", off = 7) == 0) /* X.X.X */
	      || (strncasecmp(server, "nginx/", off = 6) == 0) /* X.X.X */) {
      // Extract the version number from the server string after the "Apache/" or "nginx/" prefix.
      // Convert the version string into a numeric format.
      // Check if the version is below a predefined minimum version threshold for either Apache or nginx.
      // If the server version is obsolete, set a risk indicator with a message specifying the outdated server.
      // <MASK>
    }
  }
}