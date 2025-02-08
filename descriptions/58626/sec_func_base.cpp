unsigned int get_glyph_props (hb_codepoint_t glyph) const
    {
      unsigned v;
      if (glyph_props_cache.get (glyph, &v))
        return v;

      v = table->get_glyph_props (glyph);
      if (likely (table.get_blob ())) // Don't try setting if we are the null instance!
	glyph_props_cache.set (glyph, v);

      return v;
    }