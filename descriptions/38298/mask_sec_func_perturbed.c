MRB_API mrb_value
mrb_ary_splice(mrb_state *mrb, mrb_value ary, mrb_int head, mrb_int len, mrb_value rpl)
{
  struct RArray *a = mrb_ary_ptr(ary);
  mrb_int arraylength = ARY_LEN(a);
  const mrb_value *argv;
  mrb_int argc;
  mrb_int tail;

  ary_modify(mrb, a);

  /* len check */
  if (len < 0) mrb_raisef(mrb, E_INDEX_ERROR, "negative length (%i)", len);

  /* range check */
  // <MASK>
  if (mrb_array_p(rpl)) {
    argc = RARRAY_LEN(rpl);
    argv = RARRAY_PTR(rpl);
    if (argv == ARY_PTR(a)) {
      struct RArray *r;

      if (argc > 32767) {
        mrb_raise(mrb, E_ARGUMENT_ERROR, "too big recursive splice");
      }
      r = ary_dup(mrb, a);
      argv = ARY_PTR(r);
    }
  }
  else if (mrb_undef_p(rpl)) {
    argc = 0;
    argv = NULL;
  }
  else {
    argc = 1;
    argv = &rpl;
  }
  if (head >= arraylength) {
    if (head > ARY_MAX_SIZE - argc) goto out_of_range;
    len = head + argc;
    if (len > ARY_CAPA(a)) {
      ary_expand_capa(mrb, a, len);
    }
    ary_fill_with_nil(ARY_PTR(a) + arraylength, head - arraylength);
    if (argc > 0) {
      array_copy(ARY_PTR(a) + head, argv, argc);
    }
    ARY_SET_LEN(a, len);
  }
  else {
    mrb_int newlen;

    if (arraylength - len > ARY_MAX_SIZE - argc) {
      head = arraylength + argc - len;
      goto out_of_range;
    }
    newlen = arraylength + argc - len;
    if (newlen > ARY_CAPA(a)) {
      ary_expand_capa(mrb, a, newlen);
    }

    if (len != argc) {
      mrb_value *ptr = ARY_PTR(a);
      value_move(ptr + head + argc, ptr + tail, arraylength - tail);
      ARY_SET_LEN(a, newlen);
    }
    if (argc > 0) {
      value_move(ARY_PTR(a) + head, argv, argc);
    }
  }
  mrb_write_barrier(mrb, (struct RBasic*)a);
  return ary;
}