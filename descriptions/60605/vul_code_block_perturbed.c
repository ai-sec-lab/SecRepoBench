//case: filename="file_name
	    flow->http.filename = ndpi_malloc(filename_len);
	    if(flow->http.filename != NULL) {
	      strncpy(flow->http.filename, (char*)packet->content_disposition_line.ptr+attachment_len+1, filename_len-1);
	      flow->http.filename[filename_len-1] = '\0';
	    }