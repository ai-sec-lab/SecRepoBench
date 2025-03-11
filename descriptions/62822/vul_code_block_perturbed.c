CHECK_UNPACK_LEN(mrbstate, slen, ary);

  const char *sptr0 = (const char*)src;
  const char *sptr = sptr0;
  if (count == -1)
    count = slen * 8;

  mrb_value dst = mrb_str_new(mrbstate, NULL, count);
  char *dptr = RSTRING_PTR(dst);
  const char *dptr0 = dptr;
  int bits = 0;

  for (int i=0; i<count; i++) {
    if (flags & PACK_FLAG_LSB) {
      if (i & 7) bits >>= 1;
      else bits = (unsigned char)*sptr++;
      *dptr++ = (bits & 1) ? '1' : '0';
    }
    else {
      if (i & 7) bits <<= 1;
      else bits = (unsigned char)*sptr++;
      *dptr++ = (bits & 128) ? '1' : '0';
    }
  }