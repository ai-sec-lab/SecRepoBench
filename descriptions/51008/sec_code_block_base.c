memset (PRIV (dst_ptr_offsets) + PRIV (dst_ptr_offsets_count), 0,
	      (loc - PRIV (dst_ptr_offsets_count)) * sizeof (unsigned int));
      PRIV (dst_ptr_offsets_count) = loc + 1;