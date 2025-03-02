if (off_pos < 0 || off_pos > INT64_MAX - BLOSC_EXTENDED_HEADER_LENGTH ||
        off_pos + BLOSC_EXTENDED_HEADER_LENGTH > frame->len) {
      BLOSC_TRACE_ERROR("Cannot read the offsets outside of frame boundary.");
      return NULL;
    }
    // For in-memory frames, the coffset is just one pointer away
    uint8_t* off_start = frame->cframe + off_pos;
    if (off_cbytes != NULL) {
      rc = blosc2_cbuffer_sizes(off_start, NULL, &chunk_cbytes, NULL);
      if (rc < 0) {
        return NULL;
      }
      *off_cbytes = (int32_t)chunk_cbytes;
      if (*off_cbytes < 0 || *off_cbytes > frame->len) {
        BLOSC_TRACE_ERROR("Cannot read the cbytes outside of frame boundary.");
        return NULL;
      }
    }
    return off_start;