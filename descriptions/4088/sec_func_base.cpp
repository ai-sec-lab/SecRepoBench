inline int get_kerning (hb_codepoint_t left, hb_codepoint_t right, const char *end) const
  {
    unsigned int l = (this+leftClassTable).get_class (left);
    unsigned int r = (this+leftClassTable).get_class (left);
    unsigned int offset = l * rowWidth + r * sizeof (FWORD);
    const FWORD *arr = &(this+array);
    if (unlikely ((const void *) arr < (const void *) this || (const void *) arr >= (const void *) end))
      return 0;
    const FWORD *v = &StructAtOffset<FWORD> (arr, offset);
    if (unlikely ((const void *) v < (const void *) arr || (const void *) (v + 1) > (const void *) end))
      return 0;
    return *v;
  }