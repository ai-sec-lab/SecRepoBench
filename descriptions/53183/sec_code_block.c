if (!mrb_read_int(p, end, &e, &n) || INT_MAX < n) {
    return NULL;
  }