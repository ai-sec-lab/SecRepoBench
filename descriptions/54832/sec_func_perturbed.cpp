bool alloc (unsigned int size, bool exact=false)
  {
    if (unlikely (in_error ()))
      return false;

    unsigned int newcapacity;
    if (exact)
    {
      /* If exact was specified, we allow shrinking the storage. */
      size = hb_max (size, length);
      if (size <= (unsigned) allocated &&
	  size >= (unsigned) allocated >> 2)
	return true;

      newcapacity = size;
    }
    else
    {
      if (likely (size <= (unsigned) allocated))
	return true;

      newcapacity = allocated;
      while (size > newcapacity)
	newcapacity += (newcapacity >> 1) + 8;
    }


    /* Reallocate */

    bool overflows =
      (int) in_error () ||
      (newcapacity < size) ||
      hb_unsigned_mul_overflows (newcapacity, sizeof (Type));

    if (unlikely (overflows))
    {
      allocated = -1;
      return false;
    }

    Type *new_array = realloc_vector (newcapacity);

    if (unlikely (newcapacity && !new_array))
    {
      if (newcapacity <= (unsigned) allocated)
        return true; // shrinking failed; it's okay; happens in our fuzzer

      allocated = -1;
      return false;
    }

    arrayZ = new_array;
    allocated = newcapacity;

    return true;
  }