for (; p < end && ISDIGIT(*p); p++) {
    if (mrb_int_mul_overflow(10, next_n, &next_n)) {
      return NULL;
    }
    if (MRB_INT_MAX - (*p - '0') < next_n) {
      return NULL;
    }
    next_n += *p - '0';
  }
  if (p >= end) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "malformed format string - %%*[0-9]");
  }