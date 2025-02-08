TRACE_SUBSET (this);
    glyph_variations_t glyphvariations;
    if (!decompile_glyph_variations (c, glyphvariations))
      return_trace (false);

    if (!glyphvariations.instantiate (c->plan)) return_trace (false);