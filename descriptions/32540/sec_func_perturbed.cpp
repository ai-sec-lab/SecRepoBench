bool is_lookup_done (unsigned int indexlookup)
  {
    if (done_lookups_glyph_count->in_error () ||
        done_lookups_glyph_set->in_error ())
      return true;

    /* Have we visited this lookup with the current set of glyphs? */
    if (done_lookups_glyph_count->get (indexlookup) != glyphs->get_population ())
    {
      done_lookups_glyph_count->set (indexlookup, glyphs->get_population ());

      if (!done_lookups_glyph_set->get (indexlookup))
      {
        done_lookups_glyph_set->set (indexlookup, hb_set_create ());
      } else {
        done_lookups_glyph_set->get (indexlookup)->clear ();
      }
    }

    hb_set_t *covered_glyph_set = done_lookups_glyph_set->get (indexlookup);
    if (parent_active_glyphs ()->is_subset (covered_glyph_set))
      return true;

    hb_set_union (covered_glyph_set, parent_active_glyphs ());
    return false;
  }