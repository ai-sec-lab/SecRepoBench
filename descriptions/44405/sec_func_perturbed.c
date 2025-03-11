void ndpi_parse_packet_line_info(struct ndpi_detection_module_struct *ndpi_str, struct ndpi_flow_struct *flow) {
  u_int32_t a;
  struct ndpi_packet_struct *network_packet_struct = &ndpi_str->packet;

  if((network_packet_struct->payload_packet_len < 3) || (network_packet_struct->payload == NULL))
    return;

  if(network_packet_struct->packet_lines_parsed_complete != 0)
    return;

  network_packet_struct->packet_lines_parsed_complete = 1;
  ndpi_reset_packet_line_info(network_packet_struct);

  network_packet_struct->line[network_packet_struct->parsed_lines].ptr = network_packet_struct->payload;
  network_packet_struct->line[network_packet_struct->parsed_lines].len = 0;

  for(a = 0; ((a+1) < network_packet_struct->payload_packet_len) && (network_packet_struct->parsed_lines < NDPI_MAX_PARSE_LINES_PER_PACKET); a++) {
    if((network_packet_struct->payload[a] == 0x0d) && (network_packet_struct->payload[a+1] == 0x0a)) {
      /* If end of line char sequence CR+NL "\r\n", process line */

      if(((a + 3) < network_packet_struct->payload_packet_len)
	 && (network_packet_struct->payload[a+2] == 0x0d)
	 && (network_packet_struct->payload[a+3] == 0x0a)) {
	/* \r\n\r\n */
	int diff; /* No unsigned ! */
	u_int32_t a1 = a + 4;

	diff = network_packet_struct->payload_packet_len - a1;

	if(diff > 0) {
	  diff = ndpi_min((unsigned int)diff, sizeof(flow->initial_binary_bytes));
	  memcpy(&flow->initial_binary_bytes, &network_packet_struct->payload[a1], diff);
	  flow->initial_binary_bytes_len = diff;
	}
      }

      network_packet_struct->line[network_packet_struct->parsed_lines].len =
	(u_int16_t)(((size_t) &network_packet_struct->payload[a]) - ((size_t) network_packet_struct->line[network_packet_struct->parsed_lines].ptr));

      /* First line of a HTTP response parsing. Expected a "HTTP/1.? ???" */
      if(network_packet_struct->parsed_lines == 0 && network_packet_struct->line[0].len >= NDPI_STATICSTRING_LEN("HTTP/1.X 200 ") &&
	 strncasecmp((const char *) network_packet_struct->line[0].ptr, "HTTP/1.", NDPI_STATICSTRING_LEN("HTTP/1.")) == 0 &&
	 network_packet_struct->line[0].ptr[NDPI_STATICSTRING_LEN("HTTP/1.X ")] > '0' && /* response code between 000 and 699 */
	 network_packet_struct->line[0].ptr[NDPI_STATICSTRING_LEN("HTTP/1.X ")] < '6') {
	network_packet_struct->http_response.ptr = &network_packet_struct->line[0].ptr[NDPI_STATICSTRING_LEN("HTTP/1.1 ")];
	network_packet_struct->http_response.len = network_packet_struct->line[0].len - NDPI_STATICSTRING_LEN("HTTP/1.1 ");
	network_packet_struct->http_num_headers++;

	/* Set server HTTP response code */
	if(network_packet_struct->payload_packet_len >= 12) {
	  char buf[4];

	  /* Set server HTTP response code */
	  strncpy(buf, (char *) &network_packet_struct->payload[9], 3);
	  buf[3] = '\0';

	  flow->http.response_status_code = atoi(buf);
	  /* https://en.wikipedia.org/wiki/List_of_HTTP_status_codes */
	  if((flow->http.response_status_code < 100) || (flow->http.response_status_code > 509))
	    flow->http.response_status_code = 0; /* Out of range */
	}
      }

      /* "Server:" header line in HTTP response */
      if(network_packet_struct->line[network_packet_struct->parsed_lines].len > NDPI_STATICSTRING_LEN("Server:") + 1 &&
	 strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr,
		     "Server:", NDPI_STATICSTRING_LEN("Server:")) == 0) {
	// some stupid clients omit a space and place the servername directly after the colon
	if(network_packet_struct->line[network_packet_struct->parsed_lines].ptr[NDPI_STATICSTRING_LEN("Server:")] == ' ') {
	  network_packet_struct->server_line.ptr =
	    &network_packet_struct->line[network_packet_struct->parsed_lines].ptr[NDPI_STATICSTRING_LEN("Server:") + 1];
	  network_packet_struct->server_line.len =
	    network_packet_struct->line[network_packet_struct->parsed_lines].len - (NDPI_STATICSTRING_LEN("Server:") + 1);
	} else {
	  network_packet_struct->server_line.ptr = &network_packet_struct->line[network_packet_struct->parsed_lines].ptr[NDPI_STATICSTRING_LEN("Server:")];
	  network_packet_struct->server_line.len = network_packet_struct->line[network_packet_struct->parsed_lines].len - NDPI_STATICSTRING_LEN("Server:");
	}
	network_packet_struct->http_num_headers++;
      } else
      /* "Host:" header line in HTTP request */
      if(network_packet_struct->line[network_packet_struct->parsed_lines].len > 6 &&
	 strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Host:", 5) == 0) {
	// some stupid clients omit a space and place the hostname directly after the colon
	if(network_packet_struct->line[network_packet_struct->parsed_lines].ptr[5] == ' ') {
	  network_packet_struct->host_line.ptr = &network_packet_struct->line[network_packet_struct->parsed_lines].ptr[6];
	  network_packet_struct->host_line.len = network_packet_struct->line[network_packet_struct->parsed_lines].len - 6;
	} else {
	  network_packet_struct->host_line.ptr = &network_packet_struct->line[network_packet_struct->parsed_lines].ptr[5];
	  network_packet_struct->host_line.len = network_packet_struct->line[network_packet_struct->parsed_lines].len - 5;
	}
	network_packet_struct->http_num_headers++;
      } else
      /* "X-Forwarded-For:" header line in HTTP request. Commonly used for HTTP proxies. */
      if(network_packet_struct->line[network_packet_struct->parsed_lines].len > 17 &&
	 strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "X-Forwarded-For:", 16) == 0) {
	// some stupid clients omit a space and place the hostname directly after the colon
	if(network_packet_struct->line[network_packet_struct->parsed_lines].ptr[16] == ' ') {
	  network_packet_struct->forwarded_line.ptr = &network_packet_struct->line[network_packet_struct->parsed_lines].ptr[17];
	  network_packet_struct->forwarded_line.len = network_packet_struct->line[network_packet_struct->parsed_lines].len - 17;
	} else {
	  network_packet_struct->forwarded_line.ptr = &network_packet_struct->line[network_packet_struct->parsed_lines].ptr[16];
	  network_packet_struct->forwarded_line.len = network_packet_struct->line[network_packet_struct->parsed_lines].len - 16;
	}
	network_packet_struct->http_num_headers++;
      } else

      /* "Authorization:" header line in HTTP. */
      if(network_packet_struct->line[network_packet_struct->parsed_lines].len > 15 &&
	 (strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Authorization: ", 15) == 0)) {
	network_packet_struct->authorization_line.ptr = &network_packet_struct->line[network_packet_struct->parsed_lines].ptr[15];
	network_packet_struct->authorization_line.len = network_packet_struct->line[network_packet_struct->parsed_lines].len - 15;

	while((network_packet_struct->authorization_line.len > 0) && (network_packet_struct->authorization_line.ptr[0] == ' '))
	  network_packet_struct->authorization_line.len--, network_packet_struct->authorization_line.ptr++;
	if(network_packet_struct->authorization_line.len == 0)
	  network_packet_struct->authorization_line.ptr = NULL;

	network_packet_struct->http_num_headers++;
      } else
      /* "Accept:" header line in HTTP request. */
      if(network_packet_struct->line[network_packet_struct->parsed_lines].len > 8 &&
	 strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Accept: ", 8) == 0) {
	network_packet_struct->accept_line.ptr = &network_packet_struct->line[network_packet_struct->parsed_lines].ptr[8];
	network_packet_struct->accept_line.len = network_packet_struct->line[network_packet_struct->parsed_lines].len - 8;
	network_packet_struct->http_num_headers++;
      } else
      /* "Referer:" header line in HTTP request. */
      if(network_packet_struct->line[network_packet_struct->parsed_lines].len > 9 &&
	 strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Referer: ", 9) == 0) {
	network_packet_struct->referer_line.ptr = &network_packet_struct->line[network_packet_struct->parsed_lines].ptr[9];
	network_packet_struct->referer_line.len = network_packet_struct->line[network_packet_struct->parsed_lines].len - 9;
	network_packet_struct->http_num_headers++;
      } else
      /* "User-Agent:" header line in HTTP request. */
      if(network_packet_struct->line[network_packet_struct->parsed_lines].len > 12 &&
	  strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "User-agent: ", 12) == 0) {
	network_packet_struct->user_agent_line.ptr = &network_packet_struct->line[network_packet_struct->parsed_lines].ptr[12];
	network_packet_struct->user_agent_line.len = network_packet_struct->line[network_packet_struct->parsed_lines].len - 12;
	network_packet_struct->http_num_headers++;
      } else
      /* "Content-Encoding:" header line in HTTP response (and request?). */
      if(network_packet_struct->line[network_packet_struct->parsed_lines].len > 18 &&
	 strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Content-Encoding: ", 18) == 0) {
	network_packet_struct->http_encoding.ptr = &network_packet_struct->line[network_packet_struct->parsed_lines].ptr[18];
	network_packet_struct->http_encoding.len = network_packet_struct->line[network_packet_struct->parsed_lines].len - 18;
	network_packet_struct->http_num_headers++;
      } else
      /* "Transfer-Encoding:" header line in HTTP. */
      if(network_packet_struct->line[network_packet_struct->parsed_lines].len > 19 &&
	 strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Transfer-Encoding: ", 19) == 0) {
	network_packet_struct->http_transfer_encoding.ptr = &network_packet_struct->line[network_packet_struct->parsed_lines].ptr[19];
	network_packet_struct->http_transfer_encoding.len = network_packet_struct->line[network_packet_struct->parsed_lines].len - 19;
	network_packet_struct->http_num_headers++;
      } else
      /* "Content-Length:" header line in HTTP. */
      if(network_packet_struct->line[network_packet_struct->parsed_lines].len > 16 &&
	 strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "content-length: ", 16) == 0) {
	network_packet_struct->http_contentlen.ptr = &network_packet_struct->line[network_packet_struct->parsed_lines].ptr[16];
	network_packet_struct->http_contentlen.len = network_packet_struct->line[network_packet_struct->parsed_lines].len - 16;
	network_packet_struct->http_num_headers++;
      } else
      /* "Content-Disposition"*/
      if(network_packet_struct->line[network_packet_struct->parsed_lines].len > 21 &&
	 ((strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Content-Disposition: ", 21) == 0))) {
	network_packet_struct->content_disposition_line.ptr = &network_packet_struct->line[network_packet_struct->parsed_lines].ptr[21];
	network_packet_struct->content_disposition_line.len = network_packet_struct->line[network_packet_struct->parsed_lines].len - 21;
	network_packet_struct->http_num_headers++;
      } else
      /* "Cookie:" header line in HTTP. */
      if(network_packet_struct->line[network_packet_struct->parsed_lines].len > 8 &&
	 strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Cookie: ", 8) == 0) {
	network_packet_struct->http_cookie.ptr = &network_packet_struct->line[network_packet_struct->parsed_lines].ptr[8];
	network_packet_struct->http_cookie.len = network_packet_struct->line[network_packet_struct->parsed_lines].len - 8;
	network_packet_struct->http_num_headers++;
      } else
      /* "Origin:" header line in HTTP. */
      if(network_packet_struct->line[network_packet_struct->parsed_lines].len > 8 &&
	 strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Origin: ", 8) == 0) {
	network_packet_struct->http_origin.ptr = &network_packet_struct->line[network_packet_struct->parsed_lines].ptr[8];
	network_packet_struct->http_origin.len = network_packet_struct->line[network_packet_struct->parsed_lines].len - 8;
	network_packet_struct->http_num_headers++;
      } else
      /* "X-Session-Type:" header line in HTTP. */
      if(network_packet_struct->line[network_packet_struct->parsed_lines].len > 16 &&
	 strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "X-Session-Type: ", 16) == 0) {
	network_packet_struct->http_x_session_type.ptr = &network_packet_struct->line[network_packet_struct->parsed_lines].ptr[16];
	network_packet_struct->http_x_session_type.len = network_packet_struct->line[network_packet_struct->parsed_lines].len - 16;
	network_packet_struct->http_num_headers++;
      } else
      /* Identification and counting of other HTTP headers.
       * We consider the most common headers, but there are many others,
       * which can be seen at references below:
       * - https://tools.ietf.org/html/rfc7230
       * - https://en.wikipedia.org/wiki/List_of_HTTP_header_fields
       */
      if((network_packet_struct->line[network_packet_struct->parsed_lines].len > 6 &&
	  (strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Date: ", 6) == 0 ||
	   strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Vary: ", 6) == 0 ||
	   strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "ETag: ", 6) == 0)) ||
	 (network_packet_struct->line[network_packet_struct->parsed_lines].len > 8 &&
	  strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Pragma: ", 8) == 0) ||
	 (network_packet_struct->line[network_packet_struct->parsed_lines].len > 9 &&
	  strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Expires: ", 9) == 0) ||
	 (network_packet_struct->line[network_packet_struct->parsed_lines].len > 12 &&
	  (strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Set-Cookie: ", 12) == 0 ||
	   strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Keep-Alive: ", 12) == 0 ||
	   strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Connection: ", 12) == 0)) ||
	 (network_packet_struct->line[network_packet_struct->parsed_lines].len > 15 &&
	  (strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Last-Modified: ", 15) == 0 ||
	   strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Accept-Ranges: ", 15) == 0)) ||
	 (network_packet_struct->line[network_packet_struct->parsed_lines].len > 17 &&
	  (strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Accept-Language: ", 17) == 0 ||
	   strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Accept-Encoding: ", 17) == 0)) ||
	 (network_packet_struct->line[network_packet_struct->parsed_lines].len > 27 &&
	  strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr,
		      "Upgrade-Insecure-Requests: ", 27) == 0)) {
	/* Just count. In the future, if needed, this if can be splited to parse these headers */
	network_packet_struct->http_num_headers++;
      } else
       /* "Content-Type:" header line in HTTP. */
      if(network_packet_struct->line[network_packet_struct->parsed_lines].len > 14 &&
	 strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Content-Type: ", 14) == 0 ) {
	network_packet_struct->content_line.ptr = &network_packet_struct->line[network_packet_struct->parsed_lines].ptr[14];
	network_packet_struct->content_line.len = network_packet_struct->line[network_packet_struct->parsed_lines].len - 14;

	while((network_packet_struct->content_line.len > 0) && (network_packet_struct->content_line.ptr[0] == ' '))
	  network_packet_struct->content_line.len--, network_packet_struct->content_line.ptr++;
	if(network_packet_struct->content_line.len == 0)
	  network_packet_struct->content_line.ptr = NULL;;

	network_packet_struct->http_num_headers++;
      } else

      /* "Content-Type:" header line in HTTP AGAIN. Probably a bogus response without space after ":" */
      if((network_packet_struct->content_line.len == 0) && (network_packet_struct->line[network_packet_struct->parsed_lines].len > 13) &&
	 (strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Content-type:", 13) == 0)) {
	network_packet_struct->content_line.ptr = &network_packet_struct->line[network_packet_struct->parsed_lines].ptr[13];
	network_packet_struct->content_line.len = network_packet_struct->line[network_packet_struct->parsed_lines].len - 13;
	network_packet_struct->http_num_headers++;
      }

      if(network_packet_struct->content_line.len > 0) {
	/* application/json; charset=utf-8 */
	char separator[] = {';', '\r', '\0'};
	int i;

	for(i = 0; separator[i] != '\0'; i++) {
	  char *c = memchr((char *) network_packet_struct->content_line.ptr, separator[i], network_packet_struct->content_line.len);

	  if(c != NULL)
	    network_packet_struct->content_line.len = c - (char *) network_packet_struct->content_line.ptr;
	}
      }

      if(network_packet_struct->line[network_packet_struct->parsed_lines].len == 0) {
	network_packet_struct->empty_line_position = a;
	network_packet_struct->empty_line_position_set = 1;
      }

      if(network_packet_struct->parsed_lines >= (NDPI_MAX_PARSE_LINES_PER_PACKET - 1))
	return;

      network_packet_struct->parsed_lines++;
      network_packet_struct->line[network_packet_struct->parsed_lines].ptr = &network_packet_struct->payload[a + 2];
      network_packet_struct->line[network_packet_struct->parsed_lines].len = 0;

      a++; /* next char in the payload */
    }
  }

  if(network_packet_struct->parsed_lines >= 1) {
    network_packet_struct->line[network_packet_struct->parsed_lines].len =
      (u_int16_t)(((size_t) &network_packet_struct->payload[network_packet_struct->payload_packet_len]) -
		  ((size_t) network_packet_struct->line[network_packet_struct->parsed_lines].ptr));
    network_packet_struct->parsed_lines++;
  }
}