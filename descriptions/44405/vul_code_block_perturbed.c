{
	network_packet_struct->authorization_line.ptr = &network_packet_struct->line[network_packet_struct->parsed_lines].ptr[15];
	network_packet_struct->authorization_line.len = network_packet_struct->line[network_packet_struct->parsed_lines].len - 15;

	while((network_packet_struct->authorization_line.len > 0) && (network_packet_struct->authorization_line.ptr[0] == ' '))
	  network_packet_struct->authorization_line.len--, network_packet_struct->authorization_line.ptr++;

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

	network_packet_struct->http_num_headers++;
      } else

      /* "Content-Type:" header line in HTTP AGAIN. Probably a bogus response without space after ":" */
      if((network_packet_struct->content_line.len == 0) && (network_packet_struct->line[network_packet_struct->parsed_lines].len > 13) &&
	 (strncasecmp((const char *) network_packet_struct->line[network_packet_struct->parsed_lines].ptr, "Content-type:", 13) == 0)) {
	network_packet_struct->content_line.ptr = &network_packet_struct->line[network_packet_struct->parsed_lines].ptr[13];
	network_packet_struct->content_line.len = network_packet_struct->line[network_packet_struct->parsed_lines].len - 13;
	network_packet_struct->http_num_headers++;
      }