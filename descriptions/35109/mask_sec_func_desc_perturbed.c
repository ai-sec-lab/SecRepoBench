static const char *
get_num(mrb_state *mrb, const char *p, const char *end, int *valp)
{
  mrb_int nextnumber = (int)*valp;
  // This code parses a sequence of digits from a format string to compute an integer value.
  // It iterates over the characters in the string as long as they are digits and attempts to 
  // accumulate these digits into the next number integer.
  // After parsing the digits, check for a malformed format string, and raise an 
  // error if so.
  // <MASK>
  *valp = (int)nextnumber;
  return p;
}