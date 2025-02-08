inline bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    return_trace (likely (c->check_struct (this)) && c->check_range (&varStore, size) && varStore.sanitize (c));
  }