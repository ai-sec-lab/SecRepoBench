Type *new_array = realloc_vector (newcapacity);

    if (unlikely (newcapacity && !new_array))
    {
      if (exact and newcapacity == length)
        return true; // shrinking failed; it's okay; happens in our fuzzer

      allocated = -1;
      return false;
    }

    arrayZ = new_array;
    allocated = newcapacity;

    return true;