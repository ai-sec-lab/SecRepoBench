MRB_API void
mrb_str_modify_keep_ascii(mrb_state *mrb, struct RString *rstring)
{
  mrb_check_frozen(mrb, rstring);
  if (RSTR_SHARED_P(rstring)) {
    mrb_shared_string *shared = rstring->as.heap.aux.shared;

    if (shared->refcnt == 1 && rstring->as.heap.ptr == shared->ptr) {
      rstring->as.heap.aux.capa = shared->capa;
      rstring->as.heap.ptr[rstring->as.heap.len] = '\0';
      RSTR_UNSET_SHARED_FLAG(rstring);
      mrb_free(mrb, shared);
    }
    else {
      str_init_modifiable(mrb, rstring, rstring->as.heap.ptr, (size_t)rstring->as.heap.len);
      str_decref(mrb, shared);
    }
  }
  else if (RSTR_NOFREE_P(rstring) || RSTR_FSHARED_P(rstring)) {
    str_init_modifiable(mrb, rstring, rstring->as.heap.ptr, (size_t)rstring->as.heap.len);
  }
}