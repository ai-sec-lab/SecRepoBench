MRB_API void*
mrb_realloc(mrb_state *mrbstateptr, void *p, size_t len)
{
  void *p2;

  p2 = mrb_realloc_simple(mrbstateptr, p, len);
  if (len == 0) return p2;
  if (p2 == NULL) {
    if (mrbstateptr->gc.out_of_memory) {
      mrb_raise_nomemory(mrbstateptr);
      /* mrb_panic(mrb); */
    }
    else {
      mrbstateptr->gc.out_of_memory = TRUE;
      mrb_raise_nomemory(mrbstateptr);
    }
  }
  else {
    mrbstateptr->gc.out_of_memory = FALSE;
  }

  return p2;
}