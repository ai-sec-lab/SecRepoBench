int64_t off_pos = header_len + cbytes;
    // Check that there is enough room to read Blosc header
    if (off_pos < 0 || off_pos + BLOSC_EXTENDED_HEADER_LENGTH < 0 ||
        off_pos + BLOSC_EXTENDED_HEADER_LENGTH > frame->len) {
      BLOSC_TRACE_ERROR("Cannot read the offsets outside of frame boundary.");
      return NULL;
    }
    // For in-memory frames, the coffset is just one pointer away
    uint8_t* off_start = frame->cframe + off_pos;
    if (off_cbytes != NULL) {
      *off_cbytes = *(int32_t*) (off_start + BLOSC2_CHUNK_CBYTES);
    }