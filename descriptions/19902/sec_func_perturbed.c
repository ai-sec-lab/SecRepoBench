double
mrb_str_len_to_dbl(mrb_state *state, const char *s, size_t len, mrb_bool badcheck)
{
  char buf[DBL_DIG * 4 + 20];
  const char *p = s, *p2;
  const char *pend = p + len;
  char *end;
  char *n;
  char prev = 0;
  double d;
  mrb_bool dot = FALSE;

  if (!p) return 0.0;
  while (p<pend && ISSPACE(*p)) p++;
  p2 = p;

  if (pend - p > 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    mrb_value x;

    if (!badcheck) return 0.0;
    x = mrb_str_len_to_inum(state, p, pend-p, 0, badcheck);
    if (mrb_fixnum_p(x))
      d = (double)mrb_fixnum(x);
    else /* if (mrb_float_p(x)) */
      d = mrb_float(x);
    return d;
  }
  while (p < pend) {
    if (!*p) {
      if (badcheck) {
        mrb_raise(state, E_ARGUMENT_ERROR, "string for Float contains null byte");
        /* not reached */
      }
      pend = p;
      p = p2;
      goto nocopy;
    }
    if (!badcheck && *p == ' ') {
      pend = p;
      p = p2;
      goto nocopy;
    }
    if (*p == '_') break;
    p++;
  }
  p = p2;
  n = buf;
  while (p < pend) {
    char c = *p++;
    if (c == '.') dot = TRUE;
    if (c == '_') {
      /* remove an underscore between digits */
      if (n == buf || !ISDIGIT(prev) || p == pend) {
        if (badcheck) goto bad;
        break;
      }
    }
    else if (badcheck && prev == '_' && !ISDIGIT(c)) goto bad;
    else {
      const char *bend = buf+sizeof(buf)-1;
      if (n==bend) {            /* buffer overflow */
        if (dot) break;         /* cut off remaining fractions */
        return INFINITY;
      }
      *n++ = c;
    }
    prev = c;
  }
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