inline bool sanitize (hb_sanitize_context_t *context) const
  {
    TRACE_SANITIZE (this);
    return_trace (likely (context->check_struct (this)) && context->check_range (&varStore, size) && varStore.sanitize (context));
  }