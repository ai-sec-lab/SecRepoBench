static int
unpack_BER(mrb_state *mrb, const unsigned char *sourcedata, int srclen, mrb_value ary, unsigned int flags)
{
  mrb_int i, n = 0;
  // <MASK>
  mrb_ary_push(mrb, ary, mrb_int_value(mrb, n));
  return i;
}