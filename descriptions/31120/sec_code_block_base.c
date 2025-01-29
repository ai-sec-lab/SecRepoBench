if (rc < 0) {
    BLOSC_TRACE_ERROR("Problems retrieving a chunk offset.");
  } else if (*offset > frame->len) {
    BLOSC_TRACE_ERROR("Cannot read chunk %d outside of frame boundary.", nchunk);
    rc = BLOSC2_ERROR_READ_BUFFER;
  }