from_big(frame_len, frame_pointer + FRAME_LEN, sizeof(*frame_len));
  from_big(nbytes, frame_pointer + FRAME_NBYTES, sizeof(*nbytes));
  from_big(cbytes, frame_pointer + FRAME_CBYTES, sizeof(*cbytes));
  from_big(blocksize, frame_pointer + FRAME_BLOCKSIZE, sizeof(*blocksize));
  if (chunksize != NULL) {
    from_big(chunksize, frame_pointer + FRAME_CHUNKSIZE, sizeof(*chunksize));
  }
  if (typesize != NULL) {
    from_big(typesize, frame_pointer + FRAME_TYPESIZE, sizeof(*typesize));
  }

  if (*header_len < FRAME_HEADER_MINLEN || *header_len > *frame_len) {
    BLOSC_TRACE_ERROR("Header length is invalid or exceeds length of the frame.");
    return BLOSC2_ERROR_INVALID_HEADER;
  }