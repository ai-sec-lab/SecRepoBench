unsigned int get_glyph_props (hb_codepoint_t codepoint) const
    {
      unsigned v;
      if (glyph_props_cache.get (codepoint, &v))
        return v;

      v = table->get_glyph_props (codepoint);
      if (likely (table.get_blob ())) // Don't try setting if we are the null instance!
	glyph_props_cache.set (codepoint, v);

      return v;
    }