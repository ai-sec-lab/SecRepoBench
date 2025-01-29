scalars.resize (region_count);
	varStore->varStore.get_scalars (get_ivs (), coords, num_coords,
					&scalars[0], region_count);