mrb_int base = 10;

  mrb_get_args(state, "|i", &base);
#ifdef MRB_USE_BIGINT
  if (mrb_bigint_p(self)) {
    return mrb_bint_to_s(state, self, base);
  }
#endif
  return mrb_integer_to_str(state, self, base);