bool serialize (hb_serialize_context_t *c,
                  Iterator it)
  {
    TRACE_SERIALIZE (this);
    if (unlikely (!c->extend_min (*this))) return_trace (false);

    // <MASK>
  }