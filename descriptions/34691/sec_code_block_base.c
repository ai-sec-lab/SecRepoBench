;
  ary_modify_check(mrb, a);
  if (len == 0 || n == 0) return mrb_ary_new(mrb);
  if (n < 0) mrb_raise(mrb, E_ARGUMENT_ERROR, "negative array shift");
  if (n > len) n = len;
  val = mrb_ary_new_from_values(mrb, n, ARY_PTR(a));
  if (ARY_SHARED_P(a)) {
  L_SHIFT:
    a->as.heap.ptr+=n;
    a->as.heap.len-=n;
    return val;
  }