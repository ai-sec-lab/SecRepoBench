static int
unpack_BER(mrb_state *mrb, const unsigned char *src, int srclen, mrb_value ary, unsigned int flags)
{
  mrb_int i, n = 0;
  // This code block is responsible for unpacking a BER-compressed integer from a byte array.
  // Each byte in the source data contributes 7 bits to the final value, with the 8th bit indicating if
  // more bytes follow. If a byte's 8th bit is clear, the value is complete. The code raises an
  // exception if the integer is invalid.
  // <MASK>
  mrb_ary_push(mrb, ary, mrb_int_value(mrb, n));
  return i;
}