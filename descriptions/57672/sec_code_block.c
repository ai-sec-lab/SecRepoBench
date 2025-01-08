
    stack_copy(p, e->stack, len);
    e->stack = p;
    MRB_ENV_CLOSE(e);
    mrb_write_barrier(mrb, (struct RBasic*)e);
    return TRUE;
  