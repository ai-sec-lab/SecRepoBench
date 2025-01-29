memset (PRIV (dst_ptr_offsets) + PRIV (dst_ptr_offsets_count), 0,
	      (location - PRIV (dst_ptr_offsets_count)) * sizeof (unsigned int));
      PRIV (dst_ptr_offsets_count) = location + 1;