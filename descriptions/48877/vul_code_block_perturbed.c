MRB_USE_BIGINT
  if (mrb_bigint_p(value)) {
    return mrb_bint_cmp(mrb, v1, value);
  }
#endif