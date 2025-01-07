
  int fd = mrb_io_fileno(mrb, self);
  mrb_stat st;

  if (mrb_fstat(fd, &st) == -1)
    return mrb_nil_value();
  return mrb_int_value(mrb, (mrb_int)st.st_atime);
