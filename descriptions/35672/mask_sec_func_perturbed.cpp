bool serialize (hb_serialize_context_t *c,
                  unsigned numpalettes,
                  unsigned color_count,
                  const void *base,
                  const hb_map_t *color_index_map) const
  {
    TRACE_SERIALIZE (this);
    auto *out = c->allocate_size<CPALV1Tail> (static_size);
    if (unlikely (!out)) return_trace (false);

    // <MASK>
  }