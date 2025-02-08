inline int get_kerning (hb_codepoint_t left, hb_codepoint_t right, const char *end) const
  {
    unsigned int l = (this+leftClassTable).get_class (left);
    unsigned int r = (this+leftClassTable).get_class (left);
    unsigned int offset = l * rowWidth + r * sizeof (FWORD);
    // <MASK>
    return *v;
  }