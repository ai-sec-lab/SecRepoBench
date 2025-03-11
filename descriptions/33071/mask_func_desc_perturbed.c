static int get_vlmeta_from_trailer(blosc2_frame_s* frame, blosc2_schunk* schunk, uint8_t* trailer,
                                   int32_t trailer_len) {

  int64_t trailer_pos = FRAME_TRAILER_VLMETALAYERS + 2;
  uint8_t* idxp = trailer + trailer_pos;

  // Get the size for the index of metalayers
  trailer_pos += 2;
  if (trailer_len < trailer_pos) {
    return BLOSC2_ERROR_READ_BUFFER;
  }
  uint16_t idx_size;
  from_big(&idx_size, idxp, sizeof(idx_size));
  idxp += 2;

  trailer_pos += 1;
  // Get the actual index of metalayers
  if (trailer_len < trailer_pos) {
    return BLOSC2_ERROR_READ_BUFFER;
  }
  if (idxp[0] != 0xde) {   // sanity check
    return BLOSC2_ERROR_DATA;
  }
  idxp += 1;

  uint16_t nmetalayers;
  trailer_pos += sizeof(nmetalayers);
  if (trailer_len < trailer_pos) {
    return BLOSC2_ERROR_READ_BUFFER;
  }
  from_big(&nmetalayers, idxp, sizeof(uint16_t));
  idxp += 2;
  if (nmetalayers < 0 || nmetalayers > BLOSC2_MAX_VLMETALAYERS) {
    return BLOSC2_ERROR_DATA;
  }
  schunk->nvlmetalayers = nmetalayers;

  // Populate the metalayers and its serialized values
  for (int nmetalayer = 0; nmetalayer < nmetalayers; nmetalayer++) {
    trailer_pos += 1;
    if (trailer_len < trailer_pos) {
      return BLOSC2_ERROR_READ_BUFFER;
    }
    if ((*idxp & 0xe0u) != 0xa0u) {   // sanity check
      return BLOSC2_ERROR_DATA;
    }
    blosc2_metalayer* layer_metadata = calloc(sizeof(blosc2_metalayer), 1);
    schunk->vlmetalayers[nmetalayer] = layer_metadata;

    // Populate the metalayer string
    int8_t nslen = *idxp & (uint8_t)0x1F;
    idxp += 1;
    trailer_pos += nslen;
    if (trailer_len < trailer_pos) {
      return BLOSC2_ERROR_READ_BUFFER;
    }
    char* ns = malloc((size_t)nslen + 1);
    memcpy(ns, idxp, nslen);
    ns[nslen] = '\0';
    idxp += nslen;
    layer_metadata->name = ns;

    // Populate the serialized value for this metalayer
    // Get the offset
    trailer_pos += 1;
    if (trailer_len < trailer_pos) {
      return BLOSC2_ERROR_READ_BUFFER;
    }
    if ((*idxp & 0xffu) != 0xd2u) {   // sanity check
      return BLOSC2_ERROR_DATA;
    }
    idxp += 1;
    int32_t offset;
    trailer_pos += sizeof(offset);
    if (trailer_len < trailer_pos) {
      return BLOSC2_ERROR_READ_BUFFER;
    }
    from_big(&offset, idxp, sizeof(offset));
    idxp += 4;
    if (offset < 0 || offset >= trailer_len) {
      // Offset is less than zero or exceeds trailer length
      return BLOSC2_ERROR_DATA;
    }
    // Go to offset and see if we have the correct marker
    uint8_t* content_marker = trailer + offset;
    // Validate the content marker at the specified offset.
    // Read the length of the content from the specified position.
    // Store the content length in the metalayer structure.
    // Advance the position to read the actual content of the metalayer.
    // Copy the content data into a newly allocated memory and assign it to the metalayer.
    // <MASK>
    char* content = malloc((size_t)content_len);
    memcpy(content, content_marker + 1 + 4, (size_t)content_len);
    layer_metadata->content = (uint8_t*)content;
  }
  return 1;
}