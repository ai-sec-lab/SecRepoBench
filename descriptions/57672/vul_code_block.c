
    stack_copy(p, e->stack, len);
    e->stack = p;
    MRB_ENV_CLOSE(e);
    return TRUE;
  