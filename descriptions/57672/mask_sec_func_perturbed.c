mrb_bool
mrb_env_unshare(mrb_state *mrb_state, struct REnv *e, mrb_bool noraise)
{
  if (e == NULL) return TRUE;
  if (!MRB_ENV_ONSTACK_P(e)) return TRUE;
  if (e->cxt != mrb_state->c) return TRUE;
  if (e == CI_ENV(mrb_state->c->cibase)) return TRUE; /* for mirb */

  size_t len = (size_t)MRB_ENV_LEN(e);
  if (len == 0) {
    e->stack = NULL;
    MRB_ENV_CLOSE(e);
    return TRUE;
  }

  size_t live = mrb_state->gc.live;
  mrb_value *p = (mrb_value*)mrb_malloc_simple(mrb_state, sizeof(mrb_value)*len);
  if (live != mrb_state->gc.live && mrb_object_dead_p(mrb_state, (struct RBasic*)e)) {
    // The e object is now subject to GC inside mrb_malloc_simple().
    // Moreover, if NULL is returned due to mrb_malloc_simple() failure, simply ignore it.
    mrb_free(mrb_state, p);
    return TRUE;
  }
  else if (p) // <MASK>
}