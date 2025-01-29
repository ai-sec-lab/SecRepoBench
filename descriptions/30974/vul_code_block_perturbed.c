int64_t off_pos = header_len + cbytes;
    // Check that there is enough room to read Blosc header
    if (off_pos < 0 || off_pos + BLOSC_EXTENDED_HEADER_LENGTH < 0 ||
        off_pos + BLOSC_EXTENDED_HEADER_LENGTH > container->len) {
      BLOSC_TRACE_ERROR("Cannot read the offsets outside of frame boundary.");
      return NULL;
    }