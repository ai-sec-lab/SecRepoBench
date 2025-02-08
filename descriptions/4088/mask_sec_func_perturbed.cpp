inline int get_kerning (hb_codepoint_t leftglyph, hb_codepoint_t right, const char *end) const
  {
    unsigned int l = (this+leftClassTable).get_class (leftglyph);
    unsigned int r = (this+leftClassTable).get_class (leftglyph);
    unsigned int offset = l * rowWidth + r * sizeof (FWORD);
    // <MASK>
    return *v;
  }