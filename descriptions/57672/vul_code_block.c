if (p) {
    stack_copy(p, e->stack, len);
    e->stack = p;
    MRB_ENV_CLOSE(e);
    return TRUE;
  }
  else {
    e->stack = NULL;
    MRB_ENV_CLOSE(e);
    MRB_ENV_SET_LEN(e, 0);
    MRB_ENV_SET_BIDX(e, 0);
    if (!noraise) {
      mrb_exc_raise(mrb, mrb_obj_value(mrb->nomem_err));
    }
    return FALSE;
  }