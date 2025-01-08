if (!mrb_read_int(tptr+tmpl->idx, tptr+tlen, &e, &n)) {
        mrb_raise(mrb, E_RUNTIME_ERROR, "too big template length");
      }