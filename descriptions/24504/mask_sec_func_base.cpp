void process_blend ()
  {
    if (!seen_blend)
    {
      region_count = varStore->varStore.get_region_index_count (get_ivs ());
      if (do_blend)
      {
	// <MASK>
      }
      seen_blend = true;
    }
  }