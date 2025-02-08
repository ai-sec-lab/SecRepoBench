bool subset (hb_subset_context_t *c,
	       Iterator		    coverage,
	       unsigned		    class_count,
	       const hb_map_t	   *klass_mapping) const
  {
    TRACE_SUBSET (this);
    const hb_set_t &glyphset = *c->plan->glyphset_gsub ();

    auto *out = c->serializer->start_embed (this);
    if (unlikely (!c->serializer->extend_min (out)))  return_trace (false);

    unsigned ligature_count = 0;
    for (hb_codepoint_t gid : coverage)
    {
      // <MASK>
    }
    return_trace (this->len);
  }