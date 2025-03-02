// Read the size of the content
    int32_t content_len;
    trailer_pos += sizeof(content_len);
    if (trailer_len < trailer_pos) {
      return BLOSC2_ERROR_READ_BUFFER;
    }
    big_store(&content_len, content_marker + 1, sizeof(content_len));
    if (content_len < 0) {
      return BLOSC2_ERROR_DATA;
    }
    metalayer->content_len = content_len;

    // Finally, read the content
    trailer_pos += content_len;
    if (trailer_len < trailer_pos) {
      return BLOSC2_ERROR_READ_BUFFER;
    }