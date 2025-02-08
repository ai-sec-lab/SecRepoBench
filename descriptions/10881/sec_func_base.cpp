inline bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    if (!u.version32.sanitize (c)) return_trace (false);
    switch (u.major) {
    case 0: return_trace (u.ot.sanitize (c));
    case 1: return_trace (u.aat.sanitize (c));
    default:return_trace (true);
    }
  }