mrb_define_method(mrb, io, "close_on_exec=", io_set_close_on_exec, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, io, "close_on_exec?", io_close_on_exec_p,   MRB_ARGS_NONE());
  mrb_define_method(mrb, io, "closed?",    io_closed,     MRB_ARGS_NONE());   /* 15.2.20.5.2 */
  mrb_define_method(mrb, io, "pos",        io_pos,        MRB_ARGS_NONE());
  mrb_define_method(mrb, io, "pid",        io_pid,        MRB_ARGS_NONE());
  mrb_define_method(mrb, io, "fileno",     io_fileno,     MRB_ARGS_NONE());
  mrb_define_method(mrb, io, "write",      io_write,      MRB_ARGS_ANY());