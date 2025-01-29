
  mrb_define_method(mrb, a, "__ary_eq",        mrb_ary_eq,           MRB_ARGS_REQ(1));
  mrb_define_method(mrb, a, "__ary_cmp",       mrb_ary_cmp,          MRB_ARGS_REQ(1));
  mrb_define_method(mrb, a, "__ary_index",     mrb_ary_index_m,      MRB_ARGS_REQ(1));   /* kept for mruby-array-ext */
  mrb_define_method(mrb, a, "__svalue",        mrb_ary_svalue,       MRB_ARGS_NONE());