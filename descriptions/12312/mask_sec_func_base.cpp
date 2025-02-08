bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);

    unsigned int num_entries = 0;
    if (unlikely (!machine.sanitize (c, &num_entries))) return_trace (false);

    // <MASK>
  }