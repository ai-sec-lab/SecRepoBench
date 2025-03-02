static size_t dissect_softether_type(enum softether_value_type t,
                                     struct softether_value *v,
                                     u_int8_t const *payload,
                                     u_int16_t payload_len) {
  size_t ret = 0;
  v->type = t;
  v->value_size = 0;

  switch (t)
    {
    case VALUE_INT:
      if(payload_len < 4)
	return 0;

      v->value.value_int = ntohl(get_u_int32_t(payload, 0));
      v->value_size = sizeof(v->value.value_int);
      ret = v->value_size;
      break;

    case VALUE_DATA:
    case VALUE_STR:
    case // <MASK>

    case VALUE_INT64:
      if(payload_len < 8)
	return 0;

      v->value.value_int64 = ndpi_ntohll(get_u_int64_t(payload, 0));
      v->value_size = sizeof(v->value.value_int64);
      ret = v->value_size;
      break;
    }

  if(ret > payload_len)
    return 0;

  return ret;
}