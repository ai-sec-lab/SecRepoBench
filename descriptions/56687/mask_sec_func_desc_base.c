void
mrb_init_io(mrb_state *mrb)
{
  struct RClass *io;

  io      = mrb_define_class(mrb, "IO", mrb->object_class);
  MRB_SET_INSTANCE_TT(io, MRB_TT_CDATA);

  mrb_include_module(mrb, io, mrb_module_get(mrb, "Enumerable")); /* 15.2.20.3 */
  mrb_define_class_method(mrb, io, "_popen",  io_s_popen,   MRB_ARGS_ARG(1,2));
  mrb_define_class_method(mrb, io, "_sysclose",  io_s_sysclose, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, io, "for_fd",  io_s_for_fd,   MRB_ARGS_ARG(1,2));
  mrb_define_class_method(mrb, io, "select",  io_s_select,  MRB_ARGS_ARG(1,3));
  mrb_define_class_method(mrb, io, "sysopen", io_s_sysopen, MRB_ARGS_ARG(1,2));
#if !defined(_WIN32) && !(defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)
  mrb_define_class_method(mrb, io, "_pipe", io_s_pipe, MRB_ARGS_NONE());
#endif

  mrb_define_method(mrb, io, "initialize",      io_init, MRB_ARGS_ARG(1,2));    /* 15.2.20.5.21 (x)*/
  // Define instance methods for the IO class to manage various IO operations,
  // including copying, reading, writing, seeking, and closing the stream.
  // Additional methods provide functionality to check the readability of the stream,
  // determine if the stream is a terminal, and manage stream synchronization.
  // These methods align with standard IO behaviors and enhance the IO class with
  // capabilities such as checking if the stream is closed and managing execution state.
  // <MASK>
  mrb_define_method(mrb, io, "pid",        io_pid,        MRB_ARGS_NONE());
  mrb_define_method(mrb, io, "fileno",     io_fileno,     MRB_ARGS_NONE());
  mrb_define_method(mrb, io, "write",      io_write,      MRB_ARGS_ANY());    /* 15.2.20.5.20 */
  mrb_define_method(mrb, io, "pread",      io_pread,      MRB_ARGS_ANY());    /* ruby 2.5 feature */
  mrb_define_method(mrb, io, "pwrite",     io_pwrite,     MRB_ARGS_ANY());    /* ruby 2.5 feature */

  mrb_define_const_id(mrb, io, MRB_SYM(SEEK_SET), mrb_fixnum_value(SEEK_SET));
  mrb_define_const_id(mrb, io, MRB_SYM(SEEK_CUR), mrb_fixnum_value(SEEK_CUR));
  mrb_define_const_id(mrb, io, MRB_SYM(SEEK_END), mrb_fixnum_value(SEEK_END));

  mrb_define_method(mrb, io, "_read_buf",  io_read_buf,   MRB_ARGS_NONE());
  mrb_define_method(mrb, io, "_readchar",  io_readchar,   MRB_ARGS_NONE());
  mrb_define_class_method(mrb, io, "_bufread", io_bufread_m, MRB_ARGS_REQ(2));
}