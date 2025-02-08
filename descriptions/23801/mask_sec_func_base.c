MRB_API void*
mrb_realloc(mrb_state *mrb, void *p, size_t len)
{
  void *p2;

  p2 = mrb_realloc_simple(mrb, p, len);
  if (len == 0) return p2;
  if (p2 == NULL) {
    // <MASK>
  }
  else {
    mrb->gc.out_of_memory = FALSE;
  }

  return p2;
}