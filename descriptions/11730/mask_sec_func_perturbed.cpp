inline hb_codepoint_t get_glyph (hb_codepoint_t glyphid, unsigned int num_glyphs) const
  {
    if (format == 0)
      return u.format0.get_glyph (glyphid, num_glyphs);
    else if (format == 1)
      // <MASK>
  }