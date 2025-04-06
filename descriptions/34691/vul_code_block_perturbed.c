;
  ary_modify_check(mrb, ary);
  if (len == 0 || n == 0) return mrb_ary_new(mrb);
  if (n > len) n = len;
  val = mrb_ary_new_from_values(mrb, n, ARY_PTR(ary));
  if (ARY_SHARED_P(ary)) {
  L_SHIFT:
    ary->as.heap.ptr+=n;
    ary->as.heap.len-=n;
    return val;
  }