if (blob_result.size == 0 || !fits_in_pe(pe, offset, blob_result.length))
    {
      set_integer(i, pe->object, "number_of_user_strings");
      return;
    }

    offset += blob_result.size;
    // Avoid empty strings, which usually happen as padding at the end of the
    // stream.

    if (blob_result.length > 0)
    {
      set_sized_string(
         (char*) offset,
         blob_result.length,
         pe->object,
         "user_strings[%i]",
         i);

      offset += blob_result.length;
      i++;
    }