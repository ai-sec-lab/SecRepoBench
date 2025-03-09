char *e;
  mrb_int num;
  if (!mrb_read_int(p, end, &e, &num) || INT_MAX < num) {
    return NULL;
  }
  *valp = (int)num;