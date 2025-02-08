if (!fits_in_pe(pe, offset, us_header->Size) || *offset != 0x00)
    return;