bool instantiate (hb_subset_context_t *c) const
  {
    TRACE_SUBSET (this);
    glyph_variations_t glyph_vars;
    if (!decompile_glyph_variations (c, glyph_vars))
      return_trace (false);

    if (!glyph_vars.instantiate (c->plan)) return_trace (false);
    if (!glyph_vars.compile_bytes (c->plan->axes_index_map, c->plan->axes_old_index_tag_map))
      return_trace (false);

    unsigned axis_count = c->plan->axes_index_map.get_population ();
    unsigned num_glyphs = c->plan->num_output_glyphs ();
    auto it = hb_iter (c->plan->new_to_old_gid_list);
    return_trace (serialize (c->serializer, glyph_vars, it, axis_count, num_glyphs));
  }