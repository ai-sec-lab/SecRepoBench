static mrb_value
mrb_ary_shift_m(mrb_state *mrb, mrb_value self)
{
  struct RArray *ary = mrb_ary_ptr(self);
  mrb_int len = ARY_LEN(ary);
  mrb_int n;
  mrb_value val;

  if (mrb_get_args(mrb, "|i", &n) == 0) {
    return mrb_ary_shift(mrb, self);
  }// <MASK>
  if (len > ARY_SHIFT_SHARED_MIN) {
    ary_make_shared(mrb, ary);
    goto L_SHIFT;
  }
  else if (len == n) {
    ARY_SET_LEN(ary, 0);
  }
  else {
    mrb_value *ptr = ARY_PTR(ary);
    mrb_int size = len-n;

    while (size--) {
      *ptr = *(ptr+n);
      ++ptr;
    }
    ARY_SET_LEN(ary, len-n);
  }
  return val;
}