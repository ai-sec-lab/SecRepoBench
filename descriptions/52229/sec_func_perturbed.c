static void ndpi_check_http_server(struct ndpi_detection_module_struct *ndpi_struct,
				   struct ndpi_flow_struct *flow,
				   const char *http_server_name, u_int server_len) {
  if(server_len > 7) {
    u_int off;
  
    if(strncmp((const char *)http_server_name, "ntopng ", 7) == 0) {
      ndpi_set_detected_protocol(ndpi_struct, flow, NDPI_PROTOCOL_NTOP, NDPI_PROTOCOL_HTTP, NDPI_CONFIDENCE_DPI);
      NDPI_CLR_BIT(flow->risk, NDPI_KNOWN_PROTOCOL_ON_NON_STANDARD_PORT);
    } else if((strncasecmp(http_server_name, "Apache/", off = 7) == 0) /* X.X.X */
	      || (strncasecmp(http_server_name, "nginx/", off = 6) == 0) /* X.X.X */) {
      u_int i, j, a, b, c;
      char buf[16] = { '\0' };

      for(i=off, j=0; (i<server_len) && (j<sizeof(buf)-1)
	    && (isdigit(http_server_name[i]) || (http_server_name[i] == '.')); i++)
	buf[j++] = http_server_name[i];      

      if(sscanf(buf, "%d.%d.%d", &a, &b, &c) == 3) {
	u_int32_t version = (a * 1000000) + (b * 1000) + c;
	char msg[64];
	
	if((off == 7) && (version < MIN_APACHE_VERSION)) {
	  snprintf(msg, sizeof(msg), "Obsolete Apache server %s", buf);
	  ndpi_set_risk(ndpi_struct, flow, NDPI_HTTP_OBSOLETE_SERVER, msg);
	} else if((off == 6) && (version < MIN_NGINX_VERSION)) {
	  snprintf(msg, sizeof(msg), "Obsolete nginx server %s", buf);
	  ndpi_set_risk(ndpi_struct, flow, NDPI_HTTP_OBSOLETE_SERVER, msg);
	}
      }
    }
  }
}