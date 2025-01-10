
      char *e;
      mrb_int n;
      if (!mrb_read_int(tptr+tmpl->idx, tptr+tlen, &e, &n) || INT_MAX < n) {
        mrb_raise(mrb, E_RUNTIME_ERROR, "too big template length");
      }
      count = (int)n;
      tmpl->idx = (int)(e - tptr);
      continue;
    