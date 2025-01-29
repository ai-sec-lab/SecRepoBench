size_t value_siz;
  struct softether_value val1, val2;
  uint8_t got_hostname = 0, got_fqdn = 0;

  if(payload_len < 4)
    return 1;

  tuple_count = ntohl(get_u_int32_t(payload, 0));
  if(tuple_count == 0 || tuple_count * 8 > payload_len)
    return 1;

  payload += 4;
  payload_len -= 4;

  value_siz = dissect_softether_type(VALUE_DATA, &val1, payload, payload_len);
  if(value_siz == 0)
    return 1;

  payload += value_siz;
  payload_len -= value_siz;

  if(strncmp(val1.value.ptr.value_str, "host_name", value_siz) == 0)
    got_hostname = 1;

  for (; tuple_count > 0; --tuple_count) {
    value_siz = dissect_softether_tuples(payload, payload_len, &val1, &val2);
    if(value_siz == 0)
      break;

    if(got_hostname == 1) {
      if(val1.type == VALUE_STR && val1.value_size > 0) {
	size_t len = ndpi_min(val1.value_size, sizeof(flow->protos.softether.hostname) - 1);
	      
	strncpy(flow->protos.softether.hostname, val1.value.ptr.value_str, len);
	flow->protos.softether.hostname[len] = '\0';
      }
	  
      got_hostname = 0;
    }
    if(got_fqdn == 1) {
      if(val1.type == VALUE_STR && val1.value_size > 0)  {
	size_t len = ndpi_min(val1.value_size, sizeof(flow->protos.softether.fqdn) - 1);
	      
	strncpy(flow->protos.softether.fqdn, val1.value.ptr.value_str, len);
	flow->protos.softether.fqdn[len] = '\0';
      }
	  
      got_fqdn = 0;
    }

    if(val2.type == VALUE_DATA && val2.value_size > 0 &&
       strncmp(val2.value.ptr.value_str, "ddns_fqdn", val2.value_size) == 0)	{
      got_fqdn = 1;
    }

    payload += value_siz;
    payload_len -= value_siz;
  }

  if(payload_len != 0 || tuple_count != 0)
    return 1;
