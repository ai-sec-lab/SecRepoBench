void
mrb_init_io(mrb_state *mrbstateptr)
{
  struct RClass *io;

  io      = mrb_define_class(mrbstateptr, "IO", mrbstateptr->object_class);
  MRB_SET_INSTANCE_TT(io, MRB_TT_CDATA);

  mrb_include_module(mrbstateptr, io, mrb_module_get(mrbstateptr, "Enumerable")); /* 15.2.20.3 */
  mrb_define_class_method(mrbstateptr, io, "_popen",  io_s_popen,   MRB_ARGS_ARG(1,2));
  mrb_define_class_method(mrbstateptr, io, "_sysclose",  io_s_sysclose, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrbstateptr, io, "for_fd",  io_s_for_fd,   MRB_ARGS_ARG(1,2));
  mrb_define_class_method(mrbstateptr, io, "select",  io_s_select,  MRB_ARGS_ARG(1,3));
  mrb_define_class_method(mrbstateptr, io, "sysopen", io_s_sysopen, MRB_ARGS_ARG(1,2));
#if !defined(_WIN32) && !(defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)
  mrb_define_class_method(mrbstateptr, io, "_pipe", io_s_pipe, MRB_ARGS_NONE());
#endif

  mrb_define_method(mrbstateptr, io, "initialize",      io_init, MRB_ARGS_ARG(1,2));    /* 15.2.20.5.21 (x)*/
  mrb_define_method(mrbstateptr, io, "initialize_copy", io_init_copy, MRB_ARGS_REQ(1));
  mrb_define_method(mrbstateptr, io, "_check_readable", io_check_readable, MRB_ARGS_NONE());
  mrb_define_method(mrbstateptr, io, "isatty",     io_isatty,     MRB_ARGS_NONE());
  mrb_define_method(mrbstateptr, io, "sync",       io_sync,       MRB_ARGS_NONE());
  mrb_define_method(mrbstateptr, io, "sync=",      io_set_sync,   MRB_ARGS_REQ(1));
  mrb_define_method(mrbstateptr, io, "sysread",    io_sysread,    MRB_ARGS_ARG(1,1));
  mrb_define_method(mrbstateptr, io, "sysseek",    io_sysseek,    MRB_ARGS_ARG(1,1));
  mrb_define_method(mrbstateptr, io, "syswrite",   io_syswrite,   MRB_ARGS_REQ(1));
  mrb_define_method(mrbstateptr, io, "seek",       io_seek,       MRB_ARGS_ARG(1,1));
  mrb_define_method(mrbstateptr, io, "close",      io_close,      MRB_ARGS_NONE());   /* 15.2.20.5.1 */
  mrb_define_method(mrbstateptr, io, "close_write",    io_close_write,       MRB_ARGS_NONE());
  mrb_define_method(mrbstateptr, io, "close_on_exec=", io_set_close_on_exec, MRB_ARGS_REQ(1));
  mrb_define_method(mrbstateptr, io, "close_on_exec?", io_close_on_exec_p,   MRB_ARGS_NONE());
  mrb_define_method(mrbstateptr, io, "closed?",    io_closed,     MRB_ARGS_NONE());   /* 15.2.20.5.2 */
  mrb_define_method(mrbstateptr, io, "pos",        io_pos,        MRB_ARGS_NONE());
  mrb_define_method(mrbstateptr, io, "pid",        io_pid,        MRB_ARGS_NONE());
  mrb_define_method(mrbstateptr, io, "fileno",     io_fileno,     MRB_ARGS_NONE());
  mrb_define_method(mrbstateptr, io, "write",      io_write,      MRB_ARGS_ANY());    /* 15.2.20.5.20 */
  mrb_define_method(mrbstateptr, io, "pread",      io_pread,      MRB_ARGS_ANY());    /* ruby 2.5 feature */
  mrb_define_method(mrbstateptr, io, "pwrite",     io_pwrite,     MRB_ARGS_ANY());    /* ruby 2.5 feature */

  mrb_define_const_id(mrbstateptr, io, MRB_SYM(SEEK_SET), mrb_fixnum_value(SEEK_SET));
  mrb_define_const_id(mrbstateptr, io, MRB_SYM(SEEK_CUR), mrb_fixnum_value(SEEK_CUR));
  mrb_define_const_id(mrbstateptr, io, MRB_SYM(SEEK_END), mrb_fixnum_value(SEEK_END));

  mrb_define_method(mrbstateptr, io, "_read_buf",  io_read_buf,   MRB_ARGS_NONE());
  mrb_define_method(mrbstateptr, io, "_readchar",  io_readchar,   MRB_ARGS_NONE());
  mrb_define_class_method(mrbstateptr, io, "_bufread", io_bufread_m, MRB_ARGS_REQ(2));
}