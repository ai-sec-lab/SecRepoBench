if (!backtrack_map)
    {
      const hb_set_t &glyphset = *subsetcontext->plan->glyphset ();
      if (!hb_all (backtrack, glyphset) ||
          !hb_all (inputX, glyphset) ||
          !hb_all (lookaheadX, glyphset))
        return_trace (false);

      copy (subsetcontext->serializer, subsetcontext->plan->glyph_map);
    }
    else
    {
      if (!hb_all (backtrack, backtrack_map) ||
          !hb_all (inputX, input_map) ||
          !hb_all (lookaheadX, lookahead_map))
        return_trace (false);
      
      copy (subsetcontext->serializer, backtrack_map, input_map, lookahead_map);
    }