double
mrb_str_len_to_dbl(mrb_state *mrb, const char *s, size_t len, mrb_bool badcheck)
{
  // Parse a string to a double value, handling various string formats.
  // 1. Trim leading whitespace from the input string.
  // 2. Check for and handle hexadecimal prefixes "0x" or "0X".
  // 3. Prepare a buffer to store cleaned input by removing underscores between digits.
  // 4. Iterate over the string, copying valid characters to the buffer.
  // 5. Handle null byte termination, and check for formatting errors when 'badcheck' is true.
  // 6. Convert the cleaned string in the buffer to a double using a library function.
  // 7. Return the parsed double value.
  // <MASK>
  *n = '\0';
  p = buf;
  pend = n;
nocopy:
  d = mrb_float_read(p, &end);
  if (p == end) {
    if (badcheck) {
bad:
      mrb_raisef(mrb, E_ARGUMENT_ERROR, "invalid string for float(%!s)", s);
      /* not reached */
    }
    return d;
  }
  if (badcheck) {
    if (!end || p == end) goto bad;
    while (end<pend && ISSPACE(*end)) end++;
    if (end<pend) goto bad;
  }
  return d;
}