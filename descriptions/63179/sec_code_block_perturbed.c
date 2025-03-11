u_int16_t needed = 12 /* 10 (billion) + CVS separator + \0 */;
    int rc;

    if(remainingspace < needed) {
      if(ndpi_extend_serializer_buffer(&serializer->buffer, needed - remainingspace) < 0)
	return(-1);     	
    }

    if(!(serializer->status.flags & NDPI_SERIALIZER_STATUS_HDR_DONE)) {
      if(ndpi_serializer_header_string(serializer, key, strlen(key)) < 0)
	return(-1);
    }

    ndpi_serialize_csv_pre(serializer);
    remainingspace = serializer->buffer.size - serializer->status.buffer.size_used;

    rc = ndpi_snprintf((char*)&serializer->buffer.data[serializer->status.buffer.size_used],
		       remainingspace, "%u", value);