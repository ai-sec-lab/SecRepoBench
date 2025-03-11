if (ret < 0) {
    BLOSC_TRACE_ERROR("Unable to get the header info from frame.");
    return -1;
  }
  int64_t trailer_offset = get_trailer_offset(frame_struct, header_len, cbytes);
  if (trailer_offset < 0) {
    BLOSC_TRACE_ERROR("Unable to get the trailer offset from frame.");
    return -1;
  }
  if (trailer_offset + FRAME_TRAILER_USERMETA_LEN_OFFSET > frame_len) {
    BLOSC_TRACE_ERROR("Invalid trailer offset exceeds frame length.");
    return -1;
  }

  // Get the size of usermeta (inside the trailer)
  int32_t usermeta_len_network;