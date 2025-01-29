collect_coverage_and_indices (new_vert_coverage, vertGlyphCoverage, 0, indices, glyphset, glyph_map);
    collect_coverage_and_indices (new_hori_coverage, horizGlyphCoverage, vertGlyphCount, indices, glyphset, glyph_map);
    
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