Type *new_array = realloc_vector (new_allocated);

    if (unlikely (new_allocated && !new_array))
    {
      if (exact and new_allocated == length)
        return true; // shrinking failed; it's okay; happens in our fuzzer

      allocated = -1;
      return false;
    }

    arrayZ = new_array;
    allocated = new_allocated;

    return true;