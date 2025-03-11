bool is_lookup_done (unsigned int indexlookup)
  {
    if (done_lookups_glyph_count->in_error () ||
        done_lookups_glyph_set->in_error ())
      return true;

    // <MASK>

    hb_set_union (covered_glyph_set, parent_active_glyphs ());
    return false;
  }