inline hb_codepoint_t get_glyph (hb_codepoint_t sid, unsigned int num_glyphs) const
  {
    if (format == 0)
      return u.format0.get_glyph (sid, num_glyphs);
    else if (format == 1)
      // <MASK>
  }