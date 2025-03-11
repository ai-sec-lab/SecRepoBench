static size_t dissect_softether_type(enum softether_value_type t,
                                     struct softether_value *v,
                                     u_int8_t const *databuffer,
                                     u_int16_t payload_len) {
  size_t ret = 0;
  v->type = t;
  v->value_size = 0;

  switch (t)
    {
    case VALUE_INT:
      if(payload_len < 4)
	return 0;

      v->value.value_int = ntohl(get_u_int32_t(databuffer, 0));
      v->value_size = sizeof(v->value.value_int);
      ret = v->value_size;
      break;

    case VALUE_DATA:
    case VALUE_STR:
    case 
    // Handle the VALUE_UNISTR case for extracting and setting the value from the buffer.
    // Check if the payload length is sufficient for the expected data size.
    // Set the pointer to the raw data starting after the size indicator.
    // Verify that the buffer value size is non-zero and does not exceed the payload length.
    // Adjust the size if handling VALUE_DATA type.
    // Update the value size and return the total size processed, including size indicator.
    // <MASK>

    case VALUE_INT64:
      if(payload_len < 8)
	return 0;

      v->value.value_int64 = ndpi_ntohll(get_u_int64_t(databuffer, 0));
      v->value_size = sizeof(v->value.value_int64);
      ret = v->value_size;
      break;
    }

  if(ret > payload_len)
    return 0;

  return ret;
}