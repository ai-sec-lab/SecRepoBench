static int
unpack_bstr(mrb_state *mrbstate, const void *src, int slen, mrb_value ary, int count, unsigned int flags)
{
  // <MASK>
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
  dst = mrb_str_resize(mrbstate, dst, (mrb_int)(dptr - dptr0));
  mrb_ary_push(mrbstate, ary, dst);
  return (int)(sptr - sptr0);
}