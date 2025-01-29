if (trailer_offset + FRAME_TRAILER_USERMETA_LEN_OFFSET > frame_len) {
    BLOSC_TRACE_ERROR("Invalid trailer offset exceeds frame length.");
    return -1;
  }

  // Get the size of usermeta (inside the trailer)
  int32_t usermeta_len_network;
  if (frame->sdata != NULL) {
    memcpy(&usermeta_len_network, frame->sdata + trailer_offset + FRAME_TRAILER_USERMETA_LEN_OFFSET, sizeof(int32_t));
  } else {
    FILE* fp = fopen(frame->fname, "rb");
    fseek(fp, trailer_offset + FRAME_TRAILER_USERMETA_LEN_OFFSET, SEEK_SET);
    size_t rbytes = fread(&usermeta_len_network, 1, sizeof(int32_t), fp);
    if (rbytes != sizeof(int32_t)) {
      BLOSC_TRACE_ERROR("Cannot access the usermeta_len out of the fileframe.");
      fclose(fp);
      return -1;
    }
    fclose(fp);
  }
  int32_t usermeta_len;
  swap_store(&usermeta_len, &usermeta_len_network, sizeof(int32_t));

  if (usermeta_len == 0) {
    *usermeta = NULL;
    return 0;
  }

  *usermeta = malloc(usermeta_len);
  if (frame->sdata != NULL) {
    memcpy(*usermeta, frame->sdata + trailer_offset + FRAME_TRAILER_USERMETA_OFFSET, usermeta_len);
  }
  else {
    FILE* fp = fopen(frame->fname, "rb+");
    fseek(fp, trailer_offset + FRAME_TRAILER_USERMETA_OFFSET, SEEK_SET);
    size_t rbytes = fread(*usermeta, 1, usermeta_len, fp);
    if (rbytes != (size_t)usermeta_len) {
      BLOSC_TRACE_ERROR("Cannot read the complete usermeta chunk in frame. %ld != %ld.",
              (long)rbytes, (long)usermeta_len);
      return -1;
    }
    fclose(fp);
  }

  return usermeta_len;