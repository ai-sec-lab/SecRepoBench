{
	packet->authorization_line.ptr = &packet->line[packet->parsed_lines].ptr[15];
	packet->authorization_line.len = packet->line[packet->parsed_lines].len - 15;

	while((packet->authorization_line.len > 0) && (packet->authorization_line.ptr[0] == ' '))
	  packet->authorization_line.len--, packet->authorization_line.ptr++;
	if(packet->authorization_line.len == 0)
	  packet->authorization_line.ptr = NULL;

	packet->http_num_headers++;
      } else
      /* "Accept:" header line in HTTP request. */
      if(packet->line[packet->parsed_lines].len > 8 &&
	 strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Accept: ", 8) == 0) {
	packet->accept_line.ptr = &packet->line[packet->parsed_lines].ptr[8];
	packet->accept_line.len = packet->line[packet->parsed_lines].len - 8;
	packet->http_num_headers++;
      } else
      /* "Referer:" header line in HTTP request. */
      if(packet->line[packet->parsed_lines].len > 9 &&
	 strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Referer: ", 9) == 0) {
	packet->referer_line.ptr = &packet->line[packet->parsed_lines].ptr[9];
	packet->referer_line.len = packet->line[packet->parsed_lines].len - 9;
	packet->http_num_headers++;
      } else
      /* "User-Agent:" header line in HTTP request. */
      if(packet->line[packet->parsed_lines].len > 12 &&
	  strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "User-agent: ", 12) == 0) {
	packet->user_agent_line.ptr = &packet->line[packet->parsed_lines].ptr[12];
	packet->user_agent_line.len = packet->line[packet->parsed_lines].len - 12;
	packet->http_num_headers++;
      } else
      /* "Content-Encoding:" header line in HTTP response (and request?). */
      if(packet->line[packet->parsed_lines].len > 18 &&
	 strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Content-Encoding: ", 18) == 0) {
	packet->http_encoding.ptr = &packet->line[packet->parsed_lines].ptr[18];
	packet->http_encoding.len = packet->line[packet->parsed_lines].len - 18;
	packet->http_num_headers++;
      } else
      /* "Transfer-Encoding:" header line in HTTP. */
      if(packet->line[packet->parsed_lines].len > 19 &&
	 strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Transfer-Encoding: ", 19) == 0) {
	packet->http_transfer_encoding.ptr = &packet->line[packet->parsed_lines].ptr[19];
	packet->http_transfer_encoding.len = packet->line[packet->parsed_lines].len - 19;
	packet->http_num_headers++;
      } else
      /* "Content-Length:" header line in HTTP. */
      if(packet->line[packet->parsed_lines].len > 16 &&
	 strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "content-length: ", 16) == 0) {
	packet->http_contentlen.ptr = &packet->line[packet->parsed_lines].ptr[16];
	packet->http_contentlen.len = packet->line[packet->parsed_lines].len - 16;
	packet->http_num_headers++;
      } else
      /* "Content-Disposition"*/
      if(packet->line[packet->parsed_lines].len > 21 &&
	 ((strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Content-Disposition: ", 21) == 0))) {
	packet->content_disposition_line.ptr = &packet->line[packet->parsed_lines].ptr[21];
	packet->content_disposition_line.len = packet->line[packet->parsed_lines].len - 21;
	packet->http_num_headers++;
      } else
      /* "Cookie:" header line in HTTP. */
      if(packet->line[packet->parsed_lines].len > 8 &&
	 strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Cookie: ", 8) == 0) {
	packet->http_cookie.ptr = &packet->line[packet->parsed_lines].ptr[8];
	packet->http_cookie.len = packet->line[packet->parsed_lines].len - 8;
	packet->http_num_headers++;
      } else
      /* "Origin:" header line in HTTP. */
      if(packet->line[packet->parsed_lines].len > 8 &&
	 strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Origin: ", 8) == 0) {
	packet->http_origin.ptr = &packet->line[packet->parsed_lines].ptr[8];
	packet->http_origin.len = packet->line[packet->parsed_lines].len - 8;
	packet->http_num_headers++;
      } else
      /* "X-Session-Type:" header line in HTTP. */
      if(packet->line[packet->parsed_lines].len > 16 &&
	 strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "X-Session-Type: ", 16) == 0) {
	packet->http_x_session_type.ptr = &packet->line[packet->parsed_lines].ptr[16];
	packet->http_x_session_type.len = packet->line[packet->parsed_lines].len - 16;
	packet->http_num_headers++;
      } else
      /* Identification and counting of other HTTP headers.
       * We consider the most common headers, but there are many others,
       * which can be seen at references below:
       * - https://tools.ietf.org/html/rfc7230
       * - https://en.wikipedia.org/wiki/List_of_HTTP_header_fields
       */
      if((packet->line[packet->parsed_lines].len > 6 &&
	  (strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Date: ", 6) == 0 ||
	   strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Vary: ", 6) == 0 ||
	   strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "ETag: ", 6) == 0)) ||
	 (packet->line[packet->parsed_lines].len > 8 &&
	  strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Pragma: ", 8) == 0) ||
	 (packet->line[packet->parsed_lines].len > 9 &&
	  strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Expires: ", 9) == 0) ||
	 (packet->line[packet->parsed_lines].len > 12 &&
	  (strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Set-Cookie: ", 12) == 0 ||
	   strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Keep-Alive: ", 12) == 0 ||
	   strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Connection: ", 12) == 0)) ||
	 (packet->line[packet->parsed_lines].len > 15 &&
	  (strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Last-Modified: ", 15) == 0 ||
	   strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Accept-Ranges: ", 15) == 0)) ||
	 (packet->line[packet->parsed_lines].len > 17 &&
	  (strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Accept-Language: ", 17) == 0 ||
	   strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Accept-Encoding: ", 17) == 0)) ||
	 (packet->line[packet->parsed_lines].len > 27 &&
	  strncasecmp((const char *) packet->line[packet->parsed_lines].ptr,
		      "Upgrade-Insecure-Requests: ", 27) == 0)) {
	/* Just count. In the future, if needed, this if can be splited to parse these headers */
	packet->http_num_headers++;
      } else
       /* "Content-Type:" header line in HTTP. */
      if(packet->line[packet->parsed_lines].len > 14 &&
	 strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Content-Type: ", 14) == 0 ) {
	packet->content_line.ptr = &packet->line[packet->parsed_lines].ptr[14];
	packet->content_line.len = packet->line[packet->parsed_lines].len - 14;

	while((packet->content_line.len > 0) && (packet->content_line.ptr[0] == ' '))
	  packet->content_line.len--, packet->content_line.ptr++;
	if(packet->content_line.len == 0)
	  packet->content_line.ptr = NULL;;

	packet->http_num_headers++;
      } else

      /* "Content-Type:" header line in HTTP AGAIN. Probably a bogus response without space after ":" */
      if((packet->content_line.len == 0) && (packet->line[packet->parsed_lines].len > 13) &&
	 (strncasecmp((const char *) packet->line[packet->parsed_lines].ptr, "Content-type:", 13) == 0)) {
	packet->content_line.ptr = &packet->line[packet->parsed_lines].ptr[13];
	packet->content_line.len = packet->line[packet->parsed_lines].len - 13;
	packet->http_num_headers++;
      }