bool subset (hb_subset_context_t *context) const
  {
    TRACE_SUBSET (this);
    const hb_set_t &glyphset = *context->plan->_glyphset_mathed;
    const hb_map_t &glyph_map = *context->plan->glyph_map;

    auto *out = context->serializer->start_embed (*this);
    if (unlikely (!context->serializer->extend_min (out))) return_trace (false);
    if (!context->serializer->check_assign (out->minConnectorOverlap, minConnectorOverlap, HB_SERIALIZE_ERROR_INT_OVERFLOW))
      return_trace (false);
    
    hb_sorted_vector_t<hb_codepoint_t> new_vert_coverage;
    hb_sorted_vector_t<hb_codepoint_t> new_hori_coverage;
    hb_set_t indices;
    collect_coverage_and_indices (new_vert_coverage, vertGlyphCoverage, 0, vertGlyphCount, indices, glyphset, glyph_map);
    collect_coverage_and_indices (new_hori_coverage, horizGlyphCoverage, vertGlyphCount, vertGlyphCount + horizGlyphCount, indices, glyphset, glyph_map);
    
    if (!context->serializer->check_assign (out->vertGlyphCount, new_vert_coverage.length, HB_SERIALIZE_ERROR_INT_OVERFLOW))
      return_trace (false);
    if (!context->serializer->check_assign (out->horizGlyphCount, new_hori_coverage.length, HB_SERIALIZE_ERROR_INT_OVERFLOW))
      return_trace (false);

    for (unsigned i : indices.iter ())
    {
      auto *o = context->serializer->embed (glyphConstruction[i]);
      if (!o) return_trace (false);
      o->serialize_subset (context, glyphConstruction[i], this);
    }

    out->vertGlyphCoverage.serialize_serialize (context->serializer, new_vert_coverage.iter ());
    out->horizGlyphCoverage.serialize_serialize (context->serializer, new_hori_coverage.iter ());
    return_trace (true);
  }