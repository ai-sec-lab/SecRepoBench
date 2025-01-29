char buf[DBL_DIG * 4 + 10];
  const char *p = s;
  const char *pend = p + len;
  char *end;
  char *n;
  char prev = 0;
  double d;

  if (!p) return 0.0;
  while (p<pend && ISSPACE(*p)) p++;

  if (pend - p > 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    mrb_value x;

    if (!badcheck) return 0.0;
    x = mrb_str_len_to_inum(mrb, p, pend-p, 0, badcheck);
    if (mrb_fixnum_p(x))
      d = (double)mrb_fixnum(x);
    else /* if (mrb_float_p(x)) */
      d = mrb_float(x);
    return d;
  }
  while (p < pend) {
    if (!*p) {
      if (badcheck && p < pend) {
        mrb_raise(mrb, E_ARGUMENT_ERROR, "string for Float contains null byte");
        /* not reached */
      }
      pend = p;
      p = s;
      goto nocopy;
    }
    if (*p == '_') break;
    p++;
  }
  p = s;
  n = buf;
  while (p < pend) {
    char c = *p++;
    if (c == '_') {
      /* remove an underscore between digits */
      if (n == buf || !ISDIGIT(prev) || p == pend) {
        if (badcheck) goto bad;
        break;
      }
    }
    else if (badcheck && prev == '_' && !ISDIGIT(c)) goto bad;
    else {
      *n++ = c;
    }
    prev = c;
  }