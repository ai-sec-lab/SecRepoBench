u_int16_t needed = 11 /* 10 (billion) + CVS separator */;
    int rc;

    if(remainingspace < needed) {
      if(ndpi_extend_serializer_buffer(&serializer->buffer, needed - remainingspace) < 0)
	return(-1);
      else
	remainingspace = serializer->buffer.size - serializer->status.buffer.size_used;
    }

    if(!(serializer->status.flags & NDPI_SERIALIZER_STATUS_HDR_DONE)) {
      if(ndpi_serializer_header_string(serializer, key, strlen(key)) < 0)
	return(-1);
    }

    ndpi_serialize_csv_pre(serializer);
    /* needed--; */

    rc = ndpi_snprintf((char*)&serializer->buffer.data[serializer->status.buffer.size_used],
		       needed, "%u", value);