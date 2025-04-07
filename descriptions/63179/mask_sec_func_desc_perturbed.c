int ndpi_serialize_string_int32(ndpi_serializer *_serializer,
				const char *key, int32_t value) {
#ifdef OPTIMIZE_CSV_SERIALIZATION
  ndpi_private_serializer *serializer = (ndpi_private_serializer*)_serializer;

  if(serializer->fmt == ndpi_serialization_format_csv) {
    /* Key is ignored */
    u_int32_t remainingspace = serializer->buffer.size - serializer->status.buffer.size_used;
    // Serialize the integer value into a CSV format string and store it in the buffer.
    // Ensure that the buffer has sufficient space, extending it if necessary.
    // Prepend a header string if the header has not already been written.
    // Adjust the buffer usage count to reflect the serialized data.
    // <MASK>

    if(rc < 0 || (u_int)rc >= remainingspace)
      return(-1);
    
    serializer->status.buffer.size_used += rc;
    
    return(0);
  } else
#endif
  return(ndpi_serialize_binary_int32(_serializer, key, strlen(key), value));
}