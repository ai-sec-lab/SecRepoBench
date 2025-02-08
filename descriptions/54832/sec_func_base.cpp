bool alloc (unsigned int size, bool exact=false)
  {
    if (unlikely (in_error ()))
      return false;

    unsigned int new_allocated;
    if (exact)
    {
      /* If exact was specified, we allow shrinking the storage. */
      size = hb_max (size, length);
      if (size <= (unsigned) allocated &&
	  size >= (unsigned) allocated >> 2)
	return true;

      new_allocated = size;
    }
    else
    {
      if (likely (size <= (unsigned) allocated))
	return true;

      new_allocated = allocated;
      while (size > new_allocated)
	new_allocated += (new_allocated >> 1) + 8;
    }


    /* Reallocate */

    bool overflows =
      (int) in_error () ||
      (new_allocated < size) ||
      hb_unsigned_mul_overflows (new_allocated, sizeof (Type));

    if (unlikely (overflows))
    {
      allocated = -1;
      return false;
    }

    Type *new_array = realloc_vector (new_allocated);

    if (unlikely (new_allocated && !new_array))
    {
      if (new_allocated <= (unsigned) allocated)
        return true; // shrinking failed; it's okay; happens in our fuzzer

      allocated = -1;
      return false;
    }

    arrayZ = new_array;
    allocated = new_allocated;

    return true;
  }