uint8_t* get_coffsets(blosc2_frame_s *frame, int32_t header_len, int64_t cbytes, int32_t *off_cbytes) {
  if (frame->coffsets != NULL) {
    if (off_cbytes != NULL) {
      *off_cbytes = *(int32_t *) (frame->coffsets + BLOSC2_CHUNK_CBYTES);
    }
    return frame->coffsets;
  }
  if (frame->cframe != NULL) {
    // <MASK>
    return off_start;
  }

  int64_t trailer_offset = get_trailer_offset(frame, header_len, true);

  if (trailer_offset < BLOSC_EXTENDED_HEADER_LENGTH || trailer_offset + FRAME_TRAILER_MINLEN > frame->len) {
    BLOSC_TRACE_ERROR("Cannot read the trailer out of the frame.");
    return NULL;
  }

  int32_t coffsets_cbytes;
  if (frame->sframe) {
    coffsets_cbytes = (int32_t) (trailer_offset - (header_len + 0));
  }
  else {
    coffsets_cbytes = (int32_t) (trailer_offset - (header_len + cbytes));
  }

  if (off_cbytes != NULL) {
    *off_cbytes = coffsets_cbytes;
  }
  FILE* fp = NULL;
  uint8_t* coffsets = malloc((size_t)coffsets_cbytes);
  if (frame->sframe) {
    fp = sframe_open_index(frame->urlpath, "rb");
    fseek(fp, header_len + 0, SEEK_SET);
  }
  else {
    fp = fopen(frame->urlpath, "rb");
    fseek(fp, header_len + cbytes, SEEK_SET);
  }
  size_t rbytes = fread(coffsets, 1, (size_t)coffsets_cbytes, fp);
  fclose(fp);
  if (rbytes != (size_t)coffsets_cbytes) {
    BLOSC_TRACE_ERROR("Cannot read the offsets out of the frame.");
    free(coffsets);
    return NULL;
  }
  frame->coffsets = coffsets;
  return coffsets;
}