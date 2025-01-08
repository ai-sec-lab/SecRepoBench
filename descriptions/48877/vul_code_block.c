if (mrb_bigint_p(v2)) {
    return mrb_bint_cmp(mrb, v1, v2);
  }