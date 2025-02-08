static bool
dst_define_location (bfd *abfd, unsigned int location)
{
  vms_debug2 ((4, "dst_define_location (%d)\n", (int)location));

  if (location > 1 << 24)
    {
      /* 16M entries ought to be plenty.  */
      bfd_set_error (bfd_error_bad_value);
      _bfd_error_handler (_("dst_define_location %u too large"), location);
      return false;
    }

  /* Grow the ptr offset table if necessary.  */
  if (location + 1 > PRIV (dst_ptr_offsets_count))
    {
      PRIV (dst_ptr_offsets)
	= bfd_realloc_or_free (PRIV (dst_ptr_offsets),
			       (location + 1) * sizeof (unsigned int));
      if (PRIV (dst_ptr_offsets) == NULL)
	return false;
      memset (PRIV (dst_ptr_offsets) + PRIV (dst_ptr_offsets_count), 0,
	      (location - PRIV (dst_ptr_offsets_count)) * sizeof (unsigned int));
      PRIV (dst_ptr_offsets_count) = location + 1;
    }

  PRIV (dst_ptr_offsets)[location] = PRIV (image_offset);
  return true;
}