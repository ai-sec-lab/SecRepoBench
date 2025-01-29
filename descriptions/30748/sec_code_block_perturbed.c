if (contentsize < 0) {
      return BLOSC2_ERROR_DATA;
    }
    metalayer->content_len = contentsize;

    // Finally, read the content
    header_pos += contentsize;
    if (header_len < header_pos) {
      return BLOSC2_ERROR_READ_BUFFER;
    }
    char* content = malloc((size_t)contentsize);
    memcpy(content, content_marker + 1 + 4, (size_t)contentsize);
    metalayer->content = (uint8_t*)content;