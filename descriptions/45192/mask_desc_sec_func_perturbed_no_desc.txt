static int
unpack_BER(mrb_state *mrb, const unsigned char *src, int srclen, mrb_value array, unsigned int flags)
{
  mrb_int i, n = 0;
  // <MASK>
  mrb_ary_push(mrb, array, mrb_int_value(mrb, n));
  return i;
}