static int
unpack_bstr(mrb_state *mrb, const void *src, int slen, mrb_value ary, int count, unsigned int flags)
{
  // Unpack the binary data from the source into a string representation of bits.
  // The binary data is read from the source pointer, `src`, and the number of bits to unpack is determined by `count`.
  // If `count` is not specified, it defaults to the length of the source data multiplied by 8.
  // Create a new string `dst` to hold the unpacked bit representation with a length of `count`.
  // Iterate over each bit in the specified range and convert it to a character '0' or '1' based on the value of each bit.
  // Handle the conversion based on the specified `flags`, where `PACK_FLAG_LSB` indicates whether to read the least significant or most significant bits first.
  // Append the resulting string to the array `ary` and return the number of bytes read from the source.
  // <MASK>
  dst = mrb_str_resize(mrb, dst, (mrb_int)(dptr - dptr0));
  mrb_ary_push(mrb, ary, dst);
  return (int)(sptr - sptr0);
}