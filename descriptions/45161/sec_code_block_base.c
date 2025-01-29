if (!fits_in_pe(pe, row_offset + matched_bits, sizeof(uint32_t)))
      return;

    num_rows = yr_le32toh(*(row_offset + matched_bits));

    // Make sure that num_rows has a reasonable value. For example
    // edc05e49dd3810be67942b983455fd43 sets a large value for number of
    // rows for the BIT_MODULE section.
    if (num_rows > 10000)
      return;

    // Those tables which exist, but that we don't care about must be