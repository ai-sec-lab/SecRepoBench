mrb_int base = 10;

  mrb_get_args(mrb, "|i", &base);
#ifdef MRB_USE_BIGINT
  if (mrb_bigint_p(self)) {
    return mrb_bint_to_s(mrb, self, base);
  }
#endif
  return mrb_integer_to_str(mrb, self, base);