if (parseresult.size == 0)
      break;

    offset += parseresult.size;
    // Avoid empty strings, which usually happen as padding at the end of the
    // stream.

    if (parseresult.length > 0 && fits_in_pe(pe, offset, parseresult.length))
    {
      set_sized_string(
         (char*) offset,
         parseresult.length,
         pe->object,
         "user_strings[%i]",
         i);

      offset += parseresult.length;
      i++;
    }