/* internal */
  mrb_define_method(mrbstateptr, krn, "__to_int",                   mrb_ensure_int_type,             MRB_ARGS_NONE());    /* internal */
  mrb_define_method(mrbstateptr, krn, "__ENCODING__",               mrb_encoding,                    MRB_ARGS_NONE());
  mrb_define_method(mrbstateptr, krn, "respond_to_missing?",        mrb_false,                       MRB_ARGS_ARG(1,1));

  mrb_include_module(mrbstateptr, mrbstateptr->object_class, mrbstateptr->kernel_module);