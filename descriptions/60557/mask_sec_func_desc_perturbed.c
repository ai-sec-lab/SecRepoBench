static ndpi_protocol_category_t ndpi_http_check_content(struct ndpi_detection_module_struct *ndpi_struct,
							struct ndpi_flow_struct *flow) {
  struct ndpi_packet_struct *http_packet_struct = &ndpi_struct->packet;

  if(http_packet_struct->content_line.len > 0) {
    u_int app_len = sizeof("application");

    if(http_packet_struct->content_line.len > app_len) {
      const char *app     = (const char *)&http_packet_struct->content_line.ptr[app_len];
      u_int app_len_avail = http_packet_struct->content_line.len-app_len;

      if(strncasecmp(app, "mpeg", app_len_avail) == 0) {
	flow->guessed_category = flow->category = NDPI_PROTOCOL_CATEGORY_STREAMING;
	return(flow->category);
      } else {
	if(app_len_avail > 3) {
	  const char** cmp_mimes = NULL;

	  switch(app[0]) {
	  case 'b': cmp_mimes = download_file_mimes_b; break;
	  case 'o': cmp_mimes = download_file_mimes_o; break;
	  case 'x': cmp_mimes = download_file_mimes_x; break;
	  }

	  if(cmp_mimes != NULL) {
	    u_int8_t i;

	    for(i = 0; cmp_mimes[i] != NULL; i++) {
	      if(strncasecmp(app, cmp_mimes[i], app_len_avail) == 0) {
		flow->guessed_category = flow->category = NDPI_PROTOCOL_CATEGORY_DOWNLOAD_FT;
		NDPI_LOG_INFO(ndpi_struct, "found executable HTTP transfer");
		break;
	      }
	    }
	  }

	  /* ***************************************** */

	  switch(app[0]) {
	  case 'e': cmp_mimes = binary_file_mimes_e; break;
	  case 'j': cmp_mimes = binary_file_mimes_j; break;
	  case 'v': cmp_mimes = binary_file_mimes_v; break;
	  case 'x': cmp_mimes = binary_file_mimes_x; break;
	  }

	  if(cmp_mimes != NULL) {
	    u_int8_t i;

	    for(i = 0; cmp_mimes[i] != NULL; i++) {
	      if(strncasecmp(app, cmp_mimes[i], app_len_avail) == 0) {
		char str[64];

		snprintf(str, sizeof(str), "Found mime exe %s", cmp_mimes[i]);
		flow->guessed_category = flow->category = NDPI_PROTOCOL_CATEGORY_DOWNLOAD_FT;
		ndpi_set_binary_application_transfer(ndpi_struct, flow, str);
		NDPI_LOG_INFO(ndpi_struct, "Found executable HTTP transfer");
	      }
	    }
	  }
	}
      }
    }

    /* check for attachment */
    if(http_packet_struct->content_disposition_line.len > 0) {
      u_int8_t attachment_len = sizeof("attachment; filename");

      if(http_packet_struct->content_disposition_line.len > attachment_len) {
	u_int8_t filename_len = http_packet_struct->content_disposition_line.len - attachment_len;
	int i;

	if(http_packet_struct->content_disposition_line.ptr[attachment_len] == '\"') 
	// Extract the filename from the content disposition line based on its format.
	// Allocate memory for the filename and copy the appropriate substring from the content disposition line.
	// Ensure the filename is null-terminated after copying.
	// <MASK>

	if(filename_len > ATTACHMENT_LEN) {
	  attachment_len += filename_len-ATTACHMENT_LEN-1;

	  if((attachment_len+ATTACHMENT_LEN) <= http_packet_struct->content_disposition_line.len) {
	    for(i = 0; binary_file_ext[i] != NULL; i++) {
	      /* Use memcmp in case content-disposition contains binary data */
	      if(memcmp(&http_packet_struct->content_disposition_line.ptr[attachment_len],
			binary_file_ext[i], ATTACHMENT_LEN) == 0) {
		char str[64];

		snprintf(str, sizeof(str), "Found file extn %s", binary_file_ext[i]);
		flow->guessed_category = flow->category = NDPI_PROTOCOL_CATEGORY_DOWNLOAD_FT;
		ndpi_set_binary_application_transfer(ndpi_struct, flow, str);
		NDPI_LOG_INFO(ndpi_struct, "found executable HTTP transfer");
		return(flow->category);
	      }
	    }
	  }
	}
      }
    }

    switch(http_packet_struct->content_line.ptr[0]) {
    case 'a':
      if(strncasecmp((const char *)http_packet_struct->content_line.ptr, "audio",
		     ndpi_min(http_packet_struct->content_line.len, 5)) == 0)
	flow->guessed_category = flow->category = NDPI_PROTOCOL_CATEGORY_MEDIA;
      break;

    case 'v':
      if(strncasecmp((const char *)http_packet_struct->content_line.ptr, "video",
		     ndpi_min(http_packet_struct->content_line.len, 5)) == 0)
	flow->guessed_category = flow->category = NDPI_PROTOCOL_CATEGORY_MEDIA;
      break;
    }
  }

  return(flow->category);
}