MRB_API void
mrb_str_modify_keep_ascii(mrb_state *mrb, struct RString *s)
{
  mrb_check_frozen(mrb, s);
  if (RSTR_SHARED_P(s)) {
    mrb_shared_string *shared = s->as.heap.aux.shared;

    if (shared->refcnt == 1 && s->as.heap.ptr == shared->ptr) // <MASK>
  }
  else if (RSTR_NOFREE_P(s) || RSTR_FSHARED_P(s)) {
    str_init_modifiable(mrb, s, s->as.heap.ptr, (size_t)s->as.heap.len);
  }
}