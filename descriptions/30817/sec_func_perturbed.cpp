bool subset (hb_subset_context_t *c,
	       Iterator		    coverage,
	       unsigned		    class_count,
	       const hb_map_t	   *classmap) const
  {
    TRACE_SUBSET (this);
    const hb_set_t &glyphset = *c->plan->glyphset_gsub ();

    auto *out = c->serializer->start_embed (this);
    if (unlikely (!c->serializer->extend_min (out)))  return_trace (false);

    unsigned ligature_count = 0;
    for (hb_codepoint_t gid : coverage)
    {
      if (ligature_count >= this->len)
        break;

      ligature_count++;
      if (!glyphset.has (gid)) continue;

      auto *matrix = out->serialize_append (c->serializer);
      if (unlikely (!matrix)) return_trace (false);

      matrix->serialize_subset (c,
				this->arrayZ[ligature_count - 1],
				this,
				class_count,
				classmap);
    }
    return_trace (this->len);
  }