MRB_API void
mrb_alias_method(mrb_state *mrb, struct RClass *c, mrb_sym aliasname, mrb_sym b)
{
  mrb_method_t m = mrb_method_search(mrb, c, b);

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
      mrb_field_write_barrier(mrb, (struct RBasic*)p, (struct RBasic*)e);
    }
  }
  mrb_define_method_raw(mrb, c, aliasname, m);
}