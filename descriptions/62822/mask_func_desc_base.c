static int
unpack_bstr(mrb_state *mrb, const void *src, int slen, mrb_value ary, int count, unsigned int flags)
{
  // This function unpacks a bit string from the source buffer into an
  // array of strings (ary). It begins by checking the length of the 
  // source (slen), ensuring there is data to process. The function 
  // initializes pointers for navigating the source and destination 
  // buffers, as well as a bit counter.
  // It defaults to processing all bits in the source (slen * 8).
  // The process iterates through each bit, shifting and checking 
  // bits from the source byte and appending either '0' or '1' to 
  // the destination string. The direction of bit processing 
  // (least-significant-bit first or most-significant-bit first) 
  // is determined by the PACK_FLAG_LSB flag. Finally, the destination 
  // string is resized to the number of processed bits and added to the 
  // result array, returning the number of processed bytes.
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
  dst = mrb_str_resize(mrb, dst, (mrb_int)(dptr - dptr0));
  mrb_ary_push(mrb, ary, dst);
  return (int)(sptr - sptr0);
}