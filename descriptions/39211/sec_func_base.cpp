bool subset (hb_subset_context_t *c) const
  {
    TRACE_SUBSET (this);
    const hb_set_t &glyphset = *c->plan->_glyphset_mathed;
    const hb_map_t &glyph_map = *c->plan->glyph_map;

    auto *out = c->serializer->start_embed (*this);
    if (unlikely (!c->serializer->extend_min (out))) return_trace (false);
    if (!c->serializer->check_assign (out->minConnectorOverlap, minConnectorOverlap, HB_SERIALIZE_ERROR_INT_OVERFLOW))
      return_trace (false);
    
    hb_sorted_vector_t<hb_codepoint_t> new_vert_coverage;
    hb_sorted_vector_t<hb_codepoint_t> new_hori_coverage;
    hb_set_t indices;
    collect_coverage_and_indices (new_vert_coverage, vertGlyphCoverage, 0, vertGlyphCount, indices, glyphset, glyph_map);
    collect_coverage_and_indices (new_hori_coverage, horizGlyphCoverage, vertGlyphCount, vertGlyphCount + horizGlyphCount, indices, glyphset, glyph_map);
    
    if (!c->serializer->check_assign (out->vertGlyphCount, new_vert_coverage.length, HB_SERIALIZE_ERROR_INT_OVERFLOW))
      return_trace (false);
    if (!c->serializer->check_assign (out->horizGlyphCount, new_hori_coverage.length, HB_SERIALIZE_ERROR_INT_OVERFLOW))
      return_trace (false);

    for (unsigned i : indices.iter ())
    {
      auto *o = c->serializer->embed (glyphConstruction[i]);
      if (!o) return_trace (false);
      o->serialize_subset (c, glyphConstruction[i], this);
    }

    out->vertGlyphCoverage.serialize_serialize (c->serializer, new_vert_coverage.iter ());
    out->horizGlyphCoverage.serialize_serialize (c->serializer, new_hori_coverage.iter ());
    return_trace (true);
  }