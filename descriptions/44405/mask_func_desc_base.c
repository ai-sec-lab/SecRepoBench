void ndpi_parse_packet_line_info(struct ndpi_detection_module_struct *ndpi_str, struct ndpi_flow_struct *flow) {
  u_int32_t a;
  struct ndpi_packet_struct *packet = &ndpi_str->packet;

  if((packet->payload_packet_len < 3) || (packet->payload == NULL))
    return;

  if(packet->packet_lines_parsed_complete != 0)
    return;

  packet->packet_lines_parsed_complete = 1;
  ndpi_reset_packet_line_info(packet);

  packet->line[packet->parsed_lines].ptr = packet->payload;
  packet->line[packet->parsed_lines].len = 0;

  for(a = 0; ((a+1) < packet->payload_packet_len) && (packet->parsed_lines < NDPI_MAX_PARSE_LINES_PER_PACKET); a++) {
    if((packet->payload[a] == 0x0d) && (packet->payload[a+1] == 0x0a)) {
      /* If end of line char sequence CR+NL "\r\n", process line */

      if(((a + 3) < packet->payload_packet_len)
	 && (packet->payload[a+2] == 0x0d)
	 && (packet->payload[a+3] == 0x0a)) {
	/* \r\n\r\n */
	int diff; /* No unsigned ! */
	u_int32_t a1 = a + 4;

	diff = packet->payload_packet_len - a1;

	if(diff > 0) {
	  diff = ndpi_min((unsigned int)diff, sizeof(flow->initial_binary_bytes));
	  memcpy(&flow->initial_binary_bytes, &packet->payload[a1], diff);
	  flow->initial_binary_bytes_len = diff;
	}
      }

      packet->line[packet->parsed_lines].len =
	(u_int16_t)(((size_t) &packet->payload[a]) - ((size_t) packet->line[packet->parsed_lines].ptr));

      /* First line of a HTTP response parsing. Expected a "HTTP/1.? ???" */
      if(packet->parsed_lines == 0 && packet->line[0].len >= NDPI_STATICSTRING_LEN("HTTP/1.X 200 ") &&
	 strncasecmp((const char *) packet->line[0].ptr, "HTTP/1.", NDPI_STATICSTRING_LEN("HTTP/1.")) == 0 &&
	 packet->line[0].ptr[NDPI_STATICSTRING_LEN("HTTP/1.X ")] > '0' && /* response code between 000 and 699 */
	 packet->line[0].ptr[NDPI_STATICSTRING_LEN("HTTP/1.X ")] < '6') {
	packet->http_response.ptr = &packet->line[0].ptr[NDPI_STATICSTRING_LEN("HTTP/1.1 ")];
	packet->http_response.len = packet->line[0].len - NDPI_STATICSTRING_LEN("HTTP/1.1 ");
	packet->http_num_headers++;

	/* Set server HTTP response code */
	if(packet->payload_packet_len >= 12) {
	  char buf[4];

	  /* Set server HTTP response code */
	  strncpy(buf, (char *) &packet->payload[9], 3);
	  buf[3] = '\0';

	  flow->http.response_status_code = atoi(buf);
	  /* https://en.wikipedia.org/wiki/List_of_HTTP_status_codes */
	  if((flow->http.response_status_code < 100) || (flow->http.response_status_code > 509))
	    flow->http.response_status_code = 0; /* Out of range */
	}
      }

      /* "Server:" header line in HTTP response */
      if(packet->line[packet->parsed_lines].len > NDPI_STATICSTRING_LEN("Server:") + 1 &&
	 strncasecmp((const char *) packet->line[packet->parsed_lines].ptr,
		     "Server:", NDPI_STATICSTRING_LEN("Server:")) == 0) {
	// some stupid clients omit a space and place the servername directly after the colon
	if(packet->line[packet->parsed_lines].ptr[NDPI_STATICSTRING_LEN("Server:")] == ' ') {
	  packet->server_line.ptr =
	    &packet->line[packet->parsed_lines].ptr[NDPI_STATICSTRING_LEN("Server:") + 1];
	  packet->server_line.len =
	    packet->line[packet->parsed_lines].len - (NDPI_STATICSTRING_LEN("Server:") + 1);
	} else {
	  packet->server_line.ptr = &packet->line[packet->parsed_lines].ptr[NDPI_STATICSTRING_LEN("Server:")];
	  packet->server_line.len = packet->line[packet->parsed_lines].len - NDPI_STATICSTRING_LEN("Server:");
	}
	packet->http_num_headers++;
      } else
      /* "Host:" header line in HTTP request */
      if(packet->line[packet->parsed_lines].len > 6 &&
	 strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Host:", 5) == 0) {
	// some stupid clients omit a space and place the hostname directly after the colon
	if(packet->line[packet->parsed_lines].ptr[5] == ' ') {
	  packet->host_line.ptr = &packet->line[packet->parsed_lines].ptr[6];
	  packet->host_line.len = packet->line[packet->parsed_lines].len - 6;
	} else {
	  packet->host_line.ptr = &packet->line[packet->parsed_lines].ptr[5];
	  packet->host_line.len = packet->line[packet->parsed_lines].len - 5;
	}
	packet->http_num_headers++;
      } else
      /* "X-Forwarded-For:" header line in HTTP request. Commonly used for HTTP proxies. */
      if(packet->line[packet->parsed_lines].len > 17 &&
	 strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "X-Forwarded-For:", 16) == 0) {
	// some stupid clients omit a space and place the hostname directly after the colon
	if(packet->line[packet->parsed_lines].ptr[16] == ' ') {
	  packet->forwarded_line.ptr = &packet->line[packet->parsed_lines].ptr[17];
	  packet->forwarded_line.len = packet->line[packet->parsed_lines].len - 17;
	} else {
	  packet->forwarded_line.ptr = &packet->line[packet->parsed_lines].ptr[16];
	  packet->forwarded_line.len = packet->line[packet->parsed_lines].len - 16;
	}
	packet->http_num_headers++;
      } else

      /* "Authorization:" header line in HTTP. */
      if(packet->line[packet->parsed_lines].len > 15 &&
	 (strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Authorization: ", 15) == 0)) 
	 // Parse specific HTTP header lines from the current packet line.
	 // Extract the "Authorization" header value, trimming any leading spaces.
	 // Recognize and parse other common HTTP header fields such as "Accept",
	 // "Referer", "User-Agent", "Content-Encoding", "Transfer-Encoding",
	 // "Content-Length", "Content-Disposition", "Cookie", "Origin",
	 // "X-Session-Type", and "Content-Type".
	 // For each recognized header, set the appropriate pointer and length fields
	 // in the packet structure to point to the header's value within the payload.
	 // Increment the count of HTTP headers in the packet structure for each
	 // recognized header.
	 // Identify unrecognized header lines by comparing them against a list of
	 // common headers and increment the header count for them as well.
	 // Specifically handle and parse the "Content-Type" header even if it lacks
	 // a space after the colon.
	 // <MASK>

      if(packet->content_line.len > 0) {
	/* application/json; charset=utf-8 */
	char separator[] = {';', '\r', '\0'};
	int i;

	for(i = 0; separator[i] != '\0'; i++) {
	  char *c = memchr((char *) packet->content_line.ptr, separator[i], packet->content_line.len);

	  if(c != NULL)
	    packet->content_line.len = c - (char *) packet->content_line.ptr;
	}
      }

      if(packet->line[packet->parsed_lines].len == 0) {
	packet->empty_line_position = a;
	packet->empty_line_position_set = 1;
      }

      if(packet->parsed_lines >= (NDPI_MAX_PARSE_LINES_PER_PACKET - 1))
	return;

      packet->parsed_lines++;
      packet->line[packet->parsed_lines].ptr = &packet->payload[a + 2];
      packet->line[packet->parsed_lines].len = 0;

      a++; /* next char in the payload */
    }
  }

  if(packet->parsed_lines >= 1) {
    packet->line[packet->parsed_lines].len =
      (u_int16_t)(((size_t) &packet->payload[packet->payload_packet_len]) -
		  ((size_t) packet->line[packet->parsed_lines].ptr));
    packet->parsed_lines++;
  }
}