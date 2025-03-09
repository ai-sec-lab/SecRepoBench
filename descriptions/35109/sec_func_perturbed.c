static const char *
get_num(mrb_state *mrb, const char *p, const char *limit, int *valp)
{
  mrb_int next_n = (int)*valp;
  for (; p < limit && ISDIGIT(*p); p++) {
    if (mrb_int_mul_overflow(10, next_n, &next_n)) {
      return NULL;
    }
    if (MRB_INT_MAX - (*p - '0') < next_n) {
      return NULL;
    }
    next_n += *p - '0';
  }
  if (next_n > INT_MAX || next_n < 0) return NULL;
  if (p >= limit) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "malformed format string - %%*[0-9]");
  }
  *valp = (int)next_n;
  return p;
}