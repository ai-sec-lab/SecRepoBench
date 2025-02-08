wordSizeCount = new_word_count | (has_long ? 0x8000u /* LONG_WORDS */ : 0);

    regionIndices.len = new_ri_count;

    if (unlikely (!c->extend_size (this, get_size (), false))) return_trace (false);

    for (r = 0; r < new_ri_count; r++)
      regionIndices[r] = region_map[src->regionIndices[ri_map[r]]];