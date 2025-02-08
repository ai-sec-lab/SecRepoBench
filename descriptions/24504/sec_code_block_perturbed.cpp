if (unlikely (!scalars.resize (region_count)))
	  set_error ();
	else
	  varStore->varStore.get_scalars (get_ivs (), coords, num_coords,
					  &scalars[0], region_count);