if(payload_len < 4)
	return 0;

      v->value.ptr.raw = databuffer + 4;
      u_int32_t siz = ntohl(get_u_int32_t(databuffer, 0));
      if(siz == 0 || payload_len < siz + 3)
	return 0;

      if(t == VALUE_DATA)
	siz--;