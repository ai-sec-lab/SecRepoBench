MRB_API void
mrb_str_modify_keep_ascii(mrb_state *mrb, struct RString *s)
{
  mrb_check_frozen(mrb, s);
  if (RSTR_SHARED_P(s)) {
    mrb_shared_string *shared = s->as.heap.aux.shared;

    if (shared->refcnt == 1 && s->as.heap.ptr == shared->ptr) {
      s->as.heap.aux.capa = shared->capa;
      s->as.heap.ptr[s->as.heap.len] = '\0';
      RSTR_UNSET_SHARED_FLAG(s);
      mrb_free(mrb, shared);
    }
    else {
      str_init_modifiable(mrb, s, s->as.heap.ptr, (size_t)s->as.heap.len);
      str_decref(mrb, shared);
    }
  }
  else if (RSTR_NOFREE_P(s) || RSTR_FSHARED_P(s)) {
    str_init_modifiable(mrb, s, s->as.heap.ptr, (size_t)s->as.heap.len);
  }
}