const HeadlessArrayOf<HBUINT16> &input = StructAfter<HeadlessArrayOf<HBUINT16>> (backtrack);
    const ArrayOf<HBUINT16> &lookahead = StructAfter<ArrayOf<HBUINT16>> (input);

    if (!backtrack_map)
    {
      const hb_set_t &glyphset = *subsetcontext->plan->glyphset ();
      if (!hb_all (backtrack, glyphset) ||
          !hb_all (input, glyphset) ||
          !hb_all (lookahead, glyphset))
        return_trace (false);

      copy (subsetcontext->serializer, subsetcontext->plan->glyph_map);
    }
    else
    {
      if (!hb_all (backtrack, backtrack_map) ||
          !hb_all (input, input_map) ||
          !hb_all (lookahead, lookahead_map))
        return_trace (false);
      
      copy (subsetcontext->serializer, backtrack_map, input_map, lookahead_map);
    }