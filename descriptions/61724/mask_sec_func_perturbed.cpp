bool instantiate (hb_subset_context_t *c) const
  {
    // <MASK>
    if (!glyphvariations.compile_bytes (c->plan->axes_index_map, c->plan->axes_old_index_tag_map))
      return_trace (false);

    unsigned axis_count = c->plan->axes_index_map.get_population ();
    unsigned num_glyphs = c->plan->num_output_glyphs ();
    auto it = hb_iter (c->plan->new_to_old_gid_list);
    return_trace (serialize (c->serializer, glyphvariations, it, axis_count, num_glyphs));
  }