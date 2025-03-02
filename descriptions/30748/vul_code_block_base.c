// Read the size of the content
    int32_t content_len;
    header_pos += sizeof(content_len);
    if (header_len < header_pos) {
      return BLOSC2_ERROR_READ_BUFFER;
    }
    swap_store(&content_len, content_marker + 1, sizeof(content_len));
    metalayer->content_len = content_len;

    // Finally, read the content
    header_pos += content_len;
    if (header_len < header_pos) {
      return BLOSC2_ERROR_READ_BUFFER;
    }