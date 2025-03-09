;
  ary_modify_check(mrbstateptr, a);
  if (len == 0 || n == 0) return mrb_ary_new(mrbstateptr);
  if (n < 0) mrb_raise(mrbstateptr, E_ARGUMENT_ERROR, "negative array shift");
  if (n > len) n = len;
  val = mrb_ary_new_from_values(mrbstateptr, n, ARY_PTR(a));
  if (ARY_SHARED_P(a)) {
  L_SHIFT:
    a->as.heap.ptr+=n;
    a->as.heap.len-=n;
    return val;
  }