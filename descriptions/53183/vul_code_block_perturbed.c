char *e;
  mrb_int n;
  if (!mrb_read_int(p, end, &e, &n)) {
    return NULL;
  }
  *value_pointer = (int)n;