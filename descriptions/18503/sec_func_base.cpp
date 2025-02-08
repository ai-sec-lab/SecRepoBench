bool subset (hb_subset_context_t *c,
               const hb_map_t *backtrack_map = nullptr,
               const hb_map_t *input_map = nullptr,
               const hb_map_t *lookahead_map = nullptr) const
  {
    TRACE_SUBSET (this);

    const HeadlessArrayOf<HBUINT16> &input = StructAfter<HeadlessArrayOf<HBUINT16>> (backtrack);
    const ArrayOf<HBUINT16> &lookahead = StructAfter<ArrayOf<HBUINT16>> (input);

    if (!backtrack_map)
    {
      const hb_set_t &glyphset = *c->plan->glyphset ();
      if (!hb_all (backtrack, glyphset) ||
          !hb_all (input, glyphset) ||
          !hb_all (lookahead, glyphset))
        return_trace (false);

      copy (c->serializer, c->plan->glyph_map);
    }
    else
    {
      if (!hb_all (backtrack, backtrack_map) ||
          !hb_all (input, input_map) ||
          !hb_all (lookahead, lookahead_map))
        return_trace (false);
      
      copy (c->serializer, backtrack_map, input_map, lookahead_map);
    }

    return_trace (true);
  }