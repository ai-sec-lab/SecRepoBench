for (; p < end && ISDIGIT(*p); p++) {
    if (mrb_int_mul_overflow(10, nextnumber, &nextnumber)) {
      return NULL;
    }
    if (MRB_INT_MAX - (*p - '0') < nextnumber) {
      return NULL;
    }
    nextnumber += *p - '0';
  }
  if (nextnumber > INT_MAX || nextnumber < 0) return NULL;
  if (p >= end) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "malformed format string - %%*[0-9]");
  }