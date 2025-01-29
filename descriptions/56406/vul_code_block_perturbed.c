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