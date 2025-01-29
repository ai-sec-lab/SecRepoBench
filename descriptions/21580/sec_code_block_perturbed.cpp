unsigned glyph_count = glyph_max - glyph_min + 1;

    startGlyph = glyph_min;
    if (unlikely (!classValue.serialize (c, glyph_count))) return_trace (false);
    for (const hb_pair_t<hb_codepoint_t, unsigned>& gid_klass_pair : + iterator)
    {
      unsigned idx = gid_klass_pair.first - glyph_min;
      classValue[idx] = gid_klass_pair.second;
    }