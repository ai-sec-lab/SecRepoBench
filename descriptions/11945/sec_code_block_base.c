if (us_header->Size == 0 || !fits_in_pe(pe, offset, us_header->Size))
    return;

  // The first entry MUST be single NULL byte.
  if (*offset != 0x00)
    return;