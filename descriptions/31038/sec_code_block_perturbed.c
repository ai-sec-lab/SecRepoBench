if (content_len < 0) {
      return BLOSC2_ERROR_DATA;
    }
    metalayer->content_len = content_len;

    // Finally, read the content
    trailer_pos += content_len;
    if (trailer_len < trailer_pos) {
      return BLOSC2_ERROR_READ_BUFFER;
    }
    char* content = malloc((size_t)content_len);
    memcpy(content, content_marker + 1 + 4, (size_t)content_len);
    metalayer->content = (uint8_t*)content;