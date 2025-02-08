bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);

    unsigned int entrycount = 0;
    if (unlikely (!machine.sanitize (c, &entrycount))) return_trace (false);

    // <MASK>
  }