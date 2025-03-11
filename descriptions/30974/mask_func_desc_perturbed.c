uint8_t* get_coffsets(blosc2_frame_s *container, int32_t header_len, int64_t cbytes, int32_t *off_cbytes) {
  if (container->coffsets != NULL) {
    if (off_cbytes != NULL) {
      *off_cbytes = *(int32_t *) (container->coffsets + BLOSC2_CHUNK_CBYTES);
    }
    return container->coffsets;
  }
  if (container->cframe != NULL) {
    // Calculate the starting position for the offsets within the frame based on header length and compressed bytes.
    // Obtain the starting pointer for the offsets in memory when the frame is stored in memory.
    // If provided, update the off_cbytes to reflect the number of bytes for the offsets.
    // <MASK>
    return off_start;
  }

  int64_t trailer_offset = get_trailer_offset(container, header_len, true);

  if (trailer_offset < BLOSC_EXTENDED_HEADER_LENGTH || trailer_offset + FRAME_TRAILER_MINLEN > container->len) {
    BLOSC_TRACE_ERROR("Cannot read the trailer out of the frame.");
    return NULL;
  }

  int32_t coffsets_cbytes;
  if (container->sframe) {
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
  if (container->sframe) {
    fp = sframe_open_index(container->urlpath, "rb");
    fseek(fp, header_len + 0, SEEK_SET);
  }
  else {
    fp = fopen(container->urlpath, "rb");
    fseek(fp, header_len + cbytes, SEEK_SET);
  }
  size_t rbytes = fread(coffsets, 1, (size_t)coffsets_cbytes, fp);
  fclose(fp);
  if (rbytes != (size_t)coffsets_cbytes) {
    BLOSC_TRACE_ERROR("Cannot read the offsets out of the frame.");
    free(coffsets);
    return NULL;
  }
  container->coffsets = coffsets;
  return coffsets;
}