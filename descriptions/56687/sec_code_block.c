
  struct mrb_io *fptr = io_get_open_fptr(mrb, io);
  off_t pos = lseek(fptr->fd, 0, SEEK_CUR);
  if (pos == -1) mrb_sys_fail(mrb, 0);

  mrb_value buf = mrb_iv_get(mrb, io, MRB_IVSYM(buf));
  return mrb_int_value(mrb, pos - RSTRING_LEN(buf));
