// If the Valid bit is not set for this table, skip it...
    if (!((yr_le64toh(tilde_header->Valid) >> bit_check) & 0x01))
      continue;

    if (!fits_in_pe(pe, row_offset + matched_bits, sizeof(uint32_t)))
      return;

    num_rows = yr_le32toh(*(row_offset + matched_bits));

    // Make sure that num_rows has a reasonable value. For example
    // edc05e49dd3810be67942b983455fd43 sets a large value for number of
    // rows for the BIT_MODULE section.
    if (num_rows > 10000)
      return;