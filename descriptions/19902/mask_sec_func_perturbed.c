double
mrb_str_len_to_dbl(mrb_state *state, const char *s, size_t len, mrb_bool badcheck)
{
  // <MASK>
  *n = '\0';
  p = buf;
  pend = n;
nocopy:
  d = mrb_float_read(p, &end);
  if (p == end) {
    if (badcheck) {
bad:
      mrb_raisef(state, E_ARGUMENT_ERROR, "invalid string for float(%!s)", s);
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