void
mrb_init_array(mrb_state *mrb)
{
  struct RClass *a;

  mrb->array_class = a = mrb_define_class(mrb, "Array", mrb->object_class);              /* 15.2.12 */
  MRB_SET_INSTANCE_TT(a, MRB_TT_ARRAY);

  mrb_define_class_method(mrb, a, "[]",        mrb_ary_s_create,     MRB_ARGS_ANY());    /* 15.2.12.4.1 */

  mrb_define_method(mrb, a, "+",               mrb_ary_plus,         MRB_ARGS_REQ(1));   /* 15.2.12.5.1  */
  mrb_define_method(mrb, a, "*",               mrb_ary_times,        MRB_ARGS_REQ(1));   /* 15.2.12.5.2  */
  mrb_define_method(mrb, a, "<<",              mrb_ary_push_m,       MRB_ARGS_REQ(1));   /* 15.2.12.5.3  */
  mrb_define_method(mrb, a, "[]",              mrb_ary_aget,         MRB_ARGS_ARG(1,1)); /* 15.2.12.5.4  */
  mrb_define_method(mrb, a, "[]=",             mrb_ary_aset,         MRB_ARGS_ARG(2,1)); /* 15.2.12.5.5  */
  mrb_define_method(mrb, a, "clear",           mrb_ary_clear_m,      MRB_ARGS_NONE());   /* 15.2.12.5.6  */
  mrb_define_method(mrb, a, "concat",          mrb_ary_concat_m,     MRB_ARGS_REQ(1));   /* 15.2.12.5.8  */
  mrb_define_method(mrb, a, "delete_at",       mrb_ary_delete_at,    MRB_ARGS_REQ(1));   /* 15.2.12.5.9  */
  mrb_define_method(mrb, a, "empty?",          mrb_ary_empty_p,      MRB_ARGS_NONE());   /* 15.2.12.5.12 */
  mrb_define_method(mrb, a, "first",           mrb_ary_first,        MRB_ARGS_OPT(1));   /* 15.2.12.5.13 */
  mrb_define_method(mrb, a, "index",           mrb_ary_index_m,      MRB_ARGS_REQ(1));   /* 15.2.12.5.14 */
  mrb_define_method(mrb, a, "initialize_copy", mrb_ary_replace_m,    MRB_ARGS_REQ(1));   /* 15.2.12.5.16 */
  mrb_define_method(mrb, a, "join",            mrb_ary_join_m,       MRB_ARGS_OPT(1));   /* 15.2.12.5.17 */
  mrb_define_method(mrb, a, "last",            mrb_ary_last,         MRB_ARGS_OPT(1));   /* 15.2.12.5.18 */
  mrb_define_method(mrb, a, "length",          mrb_ary_size,         MRB_ARGS_NONE());   /* 15.2.12.5.19 */
  mrb_define_method(mrb, a, "pop",             mrb_ary_pop,          MRB_ARGS_NONE());   /* 15.2.12.5.21 */
  mrb_define_method(mrb, a, "push",            mrb_ary_push_m,       MRB_ARGS_ANY());    /* 15.2.12.5.22 */
  mrb_define_method(mrb, a, "replace",         mrb_ary_replace_m,    MRB_ARGS_REQ(1));   /* 15.2.12.5.23 */
  mrb_define_method(mrb, a, "reverse",         mrb_ary_reverse,      MRB_ARGS_NONE());   /* 15.2.12.5.24 */
  mrb_define_method(mrb, a, "reverse!",        mrb_ary_reverse_bang, MRB_ARGS_NONE());   /* 15.2.12.5.25 */
  mrb_define_method(mrb, a, "rindex",          mrb_ary_rindex_m,     MRB_ARGS_REQ(1));   /* 15.2.12.5.26 */
  mrb_define_method(mrb, a, "shift",           mrb_ary_shift_m,      MRB_ARGS_OPT(1));   /* 15.2.12.5.27 */
  mrb_define_method(mrb, a, "size",            mrb_ary_size,         MRB_ARGS_NONE());   /* 15.2.12.5.28 */
  mrb_define_method(mrb, a, "slice",           mrb_ary_aget,         MRB_ARGS_ARG(1,1)); /* 15.2.12.5.29 */
  mrb_define_method(mrb, a, "unshift",         mrb_ary_unshift_m,    MRB_ARGS_ANY());    /* 15.2.12.5.30 */
  // <MASK>
}