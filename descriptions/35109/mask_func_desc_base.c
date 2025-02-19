static const char *
get_num(mrb_state *mrb, const char *p, const char *end, int *valp)
{
  mrb_int next_n = (int)*valp;
  // This code parses a sequence of digits from a format string to compute an integer value.
  // It iterates over the characters in the string as long as they are digits and attempts to 
  // accumulate these digits into an integer `next_n`. The accumulation process checks for 
  // overflow using `mrb_int_mul_overflow` and ensures that the resulting value will not exceed 
  // `MRB_INT_MAX`. If an overflow is detected or the integer overflows beyond `MRB_INT_MAX`, 
  // the function returns `NULL`. After parsing the digits, check for a malformed format string, 
  // and raise an error if so.
  // <MASK>
  *valp = (int)next_n;
  return p;
}