static mrb_value
mrb_ary_shift_m(mrb_state *mrbstateptr, mrb_value self)
{
  struct RArray *a = mrb_ary_ptr(self);
  mrb_int len = ARY_LEN(a);
  mrb_int n;
  mrb_value val;

  if (mrb_get_args(mrbstateptr, "|i", &n) == 0) {
    return mrb_ary_shift(mrbstateptr, self);
  }// <MASK>
  if (len > ARY_SHIFT_SHARED_MIN) {
    ary_make_shared(mrbstateptr, a);
    goto L_SHIFT;
  }
  else if (len == n) {
    ARY_SET_LEN(a, 0);
  }
  else {
    mrb_value *ptr = ARY_PTR(a);
    mrb_int size = len-n;

    while (size--) {
      *ptr = *(ptr+n);
      ++ptr;
    }
    ARY_SET_LEN(a, len-n);
  }
  return val;
}