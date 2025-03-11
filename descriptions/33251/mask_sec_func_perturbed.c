static int get_meta_from_header(blosc2_frame_s* frame, blosc2_schunk* schunk, uint8_t* header,
                                int32_t header_len) {
  int64_t header_pos = FRAME_IDX_SIZE;

  // Get the size for the index of metalayers
  uint16_t idx_size;
  header_pos += sizeof(idx_size);
  if (header_len < header_pos) {
    return BLOSC2_ERROR_READ_BUFFER;
  }
  from_big(&idx_size, header + FRAME_IDX_SIZE, sizeof(idx_size));

  // Get the actual index of metalayers
  uint8_t* metalayers_idx = header + FRAME_IDX_SIZE + 2;
  header_pos += 1;
  if (header_len < header_pos) {
    return BLOSC2_ERROR_READ_BUFFER;
  }
  if (metalayers_idx[0] != 0xde) {   // sanity check
    return BLOSC2_ERROR_DATA;
  }
  uint8_t* idxp = metalayers_idx + 1;
  uint16_t nmetalayers;
  header_pos += sizeof(nmetalayers);
  if (header_len < header_pos) {
    return BLOSC2_ERROR_READ_BUFFER;
  }
  from_big(&nmetalayers, idxp, sizeof(uint16_t));
  idxp += 2;
  if (nmetalayers < 0 || nmetalayers > BLOSC2_MAX_METALAYERS) {
    return BLOSC2_ERROR_DATA;
  }
  schunk->nmetalayers = nmetalayers;

  // Populate the metalayers and its serialized values
  for (int nmetalayer = 0; nmetalayer < nmetalayers; nmetalayer++) {
    header_pos += 1;
    if (header_len < header_pos) {
      return BLOSC2_ERROR_READ_BUFFER;
    }
    if ((*idxp & 0xe0u) != 0xa0u) {   // sanity check
      return BLOSC2_ERROR_DATA;
    }
    blosc2_metalayer* current_metalayer = calloc(sizeof(blosc2_metalayer), 1);
    schunk->metalayers[nmetalayer] = current_metalayer;

    // Populate the metalayer string
    int8_t nslen = *idxp & (uint8_t)0x1F;
    idxp += 1;
    header_pos += nslen;
    if (header_len < header_pos) {
      return BLOSC2_ERROR_READ_BUFFER;
    }
    char* ns = malloc((size_t)nslen + 1);
    memcpy(ns, idxp, nslen);
    ns[nslen] = '\0';
    idxp += nslen;
    current_metalayer->name = ns;

    // Populate the serialized value for this metalayer
    // Get the offset
    header_pos += 1;
    if (header_len < header_pos) {
      return BLOSC2_ERROR_READ_BUFFER;
    }
    if ((*idxp & 0xffu) != 0xd2u) {   // sanity check
      return BLOSC2_ERROR_DATA;
    }
    idxp += 1;
    int32_t offset;
    header_pos += sizeof(offset);
    if (header_len < header_pos) {
      return BLOSC2_ERROR_READ_BUFFER;
    }
    from_big(&offset, idxp, sizeof(offset));
    idxp += 4;
    if (offset < 0 || offset >= header_len) {
      // Offset is less than zero or exceeds header length
      return BLOSC2_ERROR_DATA;
    }
    // Go to offset and see if we have the correct marker
    uint8_t* content_marker = header + offset;
    // <MASK>
    char* content = malloc((size_t)content_len);
    memcpy(content, content_marker + 1 + 4, (size_t)content_len);
    current_metalayer->content = (uint8_t*)content;
  }

  return 1;
}