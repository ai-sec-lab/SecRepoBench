void process_blend ()
  {
    if (!seen_blend)
    {
      region_count = varStore->varStore.get_region_index_count (get_ivs ());
      if (do_blend)
      {
	if (unlikely (!scalars.resize (region_count)))
	  set_error ();
	else
	  varStore->varStore.get_scalars (get_ivs (), coords, num_coords,
					  &scalars[0], region_count);
      }
      seen_blend = true;
    }
  }