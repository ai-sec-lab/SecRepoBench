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
  mrb_define_method(mrbstateptr, file, "_atime", mrb_file_atime, MRB_ARGS_NONE());
  mrb_define_method(mrbstateptr, file, "_ctime", mrb_file_ctime, MRB_ARGS_NONE());
  mrb_define_method(mrbstateptr, file, "_mtime", mrb_file_mtime, MRB_ARGS_NONE());
  mrb_define_method(mrbstateptr, file, "size", mrb_file_size, MRB_ARGS_NONE());
  mrb_define_method(mrbstateptr, file, "truncate", mrb_file_truncate, MRB_ARGS_REQ(1));

  cnst = mrb_define_module_under_id(mrbstateptr, file, MRB_SYM(Constants));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(LOCK_SH), mrb_fixnum_value(LOCK_SH));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(LOCK_EX), mrb_fixnum_value(LOCK_EX));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(LOCK_UN), mrb_fixnum_value(LOCK_UN));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(LOCK_NB), mrb_fixnum_value(LOCK_NB));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(SEPARATOR), mrb_str_new_cstr(mrbstateptr, FILE_SEPARATOR));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(PATH_SEPARATOR), mrb_str_new_cstr(mrbstateptr, PATH_SEPARATOR));
#if defined(_WIN32) || defined(_WIN64)
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(ALT_SEPARATOR), mrb_str_new_cstr(mrbstateptr, FILE_ALT_SEPARATOR));
#else
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(ALT_SEPARATOR), mrb_nil_value());
#endif
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(NULL), mrb_str_new_cstr(mrbstateptr, NULL_FILE));

  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(RDONLY), mrb_fixnum_value(MRB_O_RDONLY));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(WRONLY), mrb_fixnum_value(MRB_O_WRONLY));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(RDWR), mrb_fixnum_value(MRB_O_RDWR));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(APPEND), mrb_fixnum_value(MRB_O_APPEND));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(CREAT), mrb_fixnum_value(MRB_O_CREAT));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(EXCL), mrb_fixnum_value(MRB_O_EXCL));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(TRUNC), mrb_fixnum_value(MRB_O_TRUNC));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(NONBLOCK), mrb_fixnum_value(MRB_O_NONBLOCK));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(NOCTTY), mrb_fixnum_value(MRB_O_NOCTTY));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(BINARY), mrb_fixnum_value(MRB_O_BINARY));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(SHARE_DELETE), mrb_fixnum_value(MRB_O_SHARE_DELETE));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(SYNC), mrb_fixnum_value(MRB_O_SYNC));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(DSYNC), mrb_fixnum_value(MRB_O_DSYNC));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(RSYNC), mrb_fixnum_value(MRB_O_RSYNC));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(NOFOLLOW), mrb_fixnum_value(MRB_O_NOFOLLOW));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(NOATIME), mrb_fixnum_value(MRB_O_NOATIME));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(DIRECT), mrb_fixnum_value(MRB_O_DIRECT));
  mrb_define_const_id(mrbstateptr, cnst, MRB_SYM(TMPFILE), mrb_fixnum_value(MRB_O_TMPFILE));
}