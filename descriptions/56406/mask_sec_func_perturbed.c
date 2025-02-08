void
mrb_init_file(mrb_state *mrbstateptr)
{
  struct RClass *io, *file, *cnst;

  io   = mrb_class_get_id(mrbstateptr, MRB_SYM(IO));
  file = mrb_define_class(mrbstateptr, "File", io);
  MRB_SET_INSTANCE_TT(file, MRB_TT_CDATA);
  mrb_define_class_method(mrbstateptr, file, "umask",  mrb_file_s_umask, MRB_ARGS_OPT(1));
  mrb_define_class_method(mrbstateptr, file, "delete", mrb_file_s_unlink, MRB_ARGS_ANY());
  mrb_define_class_method(mrbstateptr, file, "unlink", mrb_file_s_unlink, MRB_ARGS_ANY());
  mrb_define_class_method(mrbstateptr, file, "rename", mrb_file_s_rename, MRB_ARGS_REQ(2));
  mrb_define_class_method(mrbstateptr, file, "symlink", mrb_file_s_symlink, MRB_ARGS_REQ(2));
  mrb_define_class_method(mrbstateptr, file, "chmod", mrb_file_s_chmod, MRB_ARGS_REQ(1) | MRB_ARGS_REST());
  mrb_define_class_method(mrbstateptr, file, "readlink", mrb_file_s_readlink, MRB_ARGS_REQ(1));

  mrb_define_class_method(mrbstateptr, file, "dirname",   mrb_file_dirname,    MRB_ARGS_REQ(1));
  mrb_define_class_method(mrbstateptr, file, "basename",  mrb_file_basename,   MRB_ARGS_REQ(1));
  mrb_define_class_method(mrbstateptr, file, "realpath",  mrb_file_realpath,   MRB_ARGS_REQ(1)|MRB_ARGS_OPT(1));
  mrb_define_class_method(mrbstateptr, file, "_getwd",    mrb_file__getwd,     MRB_ARGS_NONE());
  mrb_define_class_method(mrbstateptr, file, "_gethome",  mrb_file__gethome,   MRB_ARGS_OPT(1));

  mrb_define_method(mrbstateptr, file, "flock", mrb_file_flock, MRB_ARGS_REQ(1));
  // <MASK>
}