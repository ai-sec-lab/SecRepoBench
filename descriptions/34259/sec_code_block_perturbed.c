if (*header_len < FRAME_HEADER_MINLEN) {
    BLOSC_TRACE_ERROR("Header length is zero or smaller than min allowed.");
    return BLOSC2_ERROR_INVALID_HEADER;
  }
  from_big(frame_len, framep + FRAME_LEN, sizeof(*frame_len));
  if (*header_len > *frame_len) {
    BLOSC_TRACE_ERROR("Header length exceeds length of the frame.");
    return BLOSC2_ERROR_INVALID_HEADER;
  }
  from_big(nbytes, framep + FRAME_NBYTES, sizeof(*nbytes));
  from_big(cbytes, framep + FRAME_CBYTES, sizeof(*cbytes));
  from_big(blocksize, framep + FRAME_BLOCKSIZE, sizeof(*blocksize));
  if (chunksize != NULL) {
    from_big(chunksize, framep + FRAME_CHUNKSIZE, sizeof(*chunksize));
  }
  if (typesize != NULL) {
    from_big(typesize, framep + FRAME_TYPESIZE, sizeof(*typesize));
    if (*typesize <= 0 || *typesize > BLOSC_MAX_TYPESIZE) {
      BLOSC_TRACE_ERROR("`typesize` is zero or greater than max allowed.");
      return BLOSC2_ERROR_INVALID_HEADER;
    }
  }