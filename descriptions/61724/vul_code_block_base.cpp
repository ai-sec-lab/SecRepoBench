TRACE_SUBSET (this);
    glyph_variations_t glyph_vars;
    if (!decompile_glyph_variations (c->plan, glyph_vars))
      return_trace (false);

    if (!glyph_vars.instantiate (c->plan)) return_trace (false);