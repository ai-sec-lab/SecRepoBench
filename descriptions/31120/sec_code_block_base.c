uint8_t *coffsets = get_coffsets(frame, header_len, cbytes, NULL);
  if (coffsets == NULL) {
    BLOSC_TRACE_ERROR("Cannot get the offset for chunk %d for the frame.", nchunk);
    return BLOSC2_ERROR_DATA;
  }

  // Get the 64-bit offset
  int rc = blosc_getitem(coffsets, nchunk, 1, offset);
  if (rc < 0) {
    BLOSC_TRACE_ERROR("Problems retrieving a chunk offset.");
  } else if (*offset > frame->len) {
    BLOSC_TRACE_ERROR("Cannot read chunk %d outside of frame boundary.", nchunk);
    rc = BLOSC2_ERROR_READ_BUFFER;
  }

  return rc;