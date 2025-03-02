/* Have we visited this lookup with the current set of glyphs? */
    if (done_lookups_glyph_count->get (lookup_index) != glyphs->get_population ())
    {
      done_lookups_glyph_count->set (lookup_index, glyphs->get_population ());

      if (!done_lookups_glyph_set->get (lookup_index))
      {
        done_lookups_glyph_set->set (lookup_index, hb_set_create ());
      } else {
        done_lookups_glyph_set->get (lookup_index)->clear ();
      }
    }

    hb_set_t *covered_glyph_set = done_lookups_glyph_set->get (lookup_index);
    if (parent_active_glyphs ()->is_subset (covered_glyph_set))
      return true;