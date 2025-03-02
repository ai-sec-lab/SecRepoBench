VALUE_UNISTR:
      if(payload_len < 4)
	return 0;

      v->value.ptr.raw = payload + 4;
      u_int32_t siz = ntohl(get_u_int32_t(payload, 0));
      if(siz == 0 || (u_int64_t)payload_len < (u_int64_t)siz + 3)
	return 0;

      if(t == VALUE_DATA)
	siz--;

      v->value_size = siz;
      ret = siz + sizeof(siz);
      break;