/* internal */
  mrb_define_method(mrb, krn, "__to_int",                   mrb_ensure_int_type,             MRB_ARGS_NONE());    /* internal */
  mrb_define_method(mrb, krn, "__ENCODING__",               mrb_encoding,                    MRB_ARGS_NONE());

  mrb_include_module(mrb, mrb->object_class, mrb->kernel_module);