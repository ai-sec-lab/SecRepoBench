const unsigned char *p = sourcedata;
  const unsigned char *e = p + srclen;

  if (srclen == 0) return 0;
  for (i=1; p<e; p++,i++) {
    if (n > (MRB_INT_MAX>>7)) {
      mrb_raise(mrb, E_RANGE_ERROR, "BER unpacking 'w' overflow");
    }
    n <<= 7;
    n |= *p & 0x7f;
    if ((*p & 0x80) == 0) break;
  }