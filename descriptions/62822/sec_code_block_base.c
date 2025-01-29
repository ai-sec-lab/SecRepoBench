CHECK_UNPACK_LEN(mrb, slen, ary);

  const char *sptr0 = (const char*)src;
  const char *sptr = sptr0;
  if (count == -1 || count > slen * 8)
    count = slen * 8;

  mrb_value dst = mrb_str_new(mrb, NULL, count);