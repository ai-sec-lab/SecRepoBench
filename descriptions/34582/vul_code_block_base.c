if (!MRB_METHOD_CFUNC_P(m)) {
    struct RProc *p = MRB_METHOD_PROC(m);

    if (MRB_PROC_ENV_P(p)) {
      MRB_PROC_ENV(p)->mid = b;
    }
    else if (p->color != MRB_GC_RED) {
      struct RClass *tc = MRB_PROC_TARGET_CLASS(p);
      struct REnv *e = (struct REnv*)mrb_obj_alloc(mrb, MRB_TT_ENV, NULL);

      e->mid = b;
      if (tc) {
        e->c = tc;
        mrb_field_write_barrier(mrb, (struct RBasic*)e, (struct RBasic*)tc);
      }
      p->e.env = e;
      p->flags |= MRB_PROC_ENVSET;
    }
  }
  mrb_define_method_raw(mrb, c, a, m);