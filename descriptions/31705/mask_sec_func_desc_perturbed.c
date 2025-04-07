int frame_get_lazychunk(blosc2_frame_s *frame, int nchunk, uint8_t **chunk, bool *needs_free) {
  int32_t header_len;
  int64_t frame_len;
  int64_t nbytes;
  int64_t cbytes;
  int32_t blocksize;
  int32_t chunksize;
  int32_t nchunks;
  int32_t typesize;
  int32_t lazy_chunk_bytes;
  int64_t offset;
  FILE* fp = NULL;

  *chunk = NULL;
  *needs_free = false;
  int rc = get_header_info(frame, &header_len, &frame_len, &nbytes, &cbytes,
                           &blocksize, &chunksize, &nchunks,
                           &typesize, NULL, NULL, NULL, NULL);
  if (rc < 0) {
    BLOSC_TRACE_ERROR("Unable to get meta info from frame.");
    return rc;
  }

  if (nchunk >= nchunks) {
    BLOSC_TRACE_ERROR("nchunk ('%d') exceeds the number of chunks "
                      "('%d') in frame.", nchunk, nchunks);
    return BLOSC2_ERROR_INVALID_PARAM;
  }

  // Get the offset to nchunk
  rc = get_coffset(frame, header_len, cbytes, nchunk, &offset);
  if (rc < 0) {
    BLOSC_TRACE_ERROR("Unable to get offset to chunk %d.", nchunk);
    return rc;
  }

  if (offset < 0) {
    // Special value
    lazy_chunk_bytes = BLOSC_EXTENDED_HEADER_LENGTH;
    rc = frame_special_chunk(offset, chunksize, typesize, chunk,
                             (int32_t)lazy_chunk_bytes, needs_free);
    goto end;
  }

  if (frame->cframe == NULL) {
    // TODO: make this portable across different endianness
    // Get info for building a lazy chunk
    int32_t chunk_nbytes;
    int32_t chunk_cbytes;
    int32_t chunk_blocksize;
    uint8_t header[BLOSC_MIN_HEADER_LENGTH];
    if (frame->sframe) {
      // The chunk is not in the frame
      fp = sframe_open_chunk(frame->urlpath, offset, "rb");
    }
    else {
      fp = fopen(frame->urlpath, "rb");
      fseek(fp, header_len + offset, SEEK_SET);
    }
    size_t rbytes = fread(header, 1, BLOSC_MIN_HEADER_LENGTH, fp);
    if (rbytes != BLOSC_MIN_HEADER_LENGTH) {
      BLOSC_TRACE_ERROR("Cannot read the header for chunk in the frame.");
      rc = BLOSC2_ERROR_FILE_READ;
      goto end;
    }
    rc = blosc2_cbuffer_sizes(header, &chunk_nbytes, &chunk_cbytes, &chunk_blocksize);
    if (rc < 0) {
      goto end;
    }
    size_t nblocks = chunk_nbytes / chunk_blocksize;
    size_t leftover_block = chunk_nbytes % chunk_blocksize;
    nblocks = leftover_block ? nblocks + 1 : nblocks;
    // Allocate space for the lazy chunk
    size_t trailer_len = sizeof(int32_t) + sizeof(int64_t) + nblocks * sizeof(int32_t);
    size_t trailer_offset = BLOSC_EXTENDED_HEADER_LENGTH + nblocks * sizeof(int32_t);
    lazy_chunk_bytes = trailer_offset + trailer_len;
    *chunk = malloc(lazy_chunk_bytes);
    *needs_free = true;

    // Read just the full header and bstarts section too (lazy partial length)
    if (frame->sframe) {
      fseek(fp, 0, SEEK_SET);
    }
    else {
      fseek(fp, header_len + offset, SEEK_SET);
    }

    rbytes = fread(*chunk, 1, trailer_offset, fp);
    if (rbytes != trailer_offset) {
      BLOSC_TRACE_ERROR("Cannot read the (lazy) chunk out of the frame.");
      rc = BLOSC2_ERROR_FILE_READ;
      goto end;
    }

    // Mark chunk as lazy
    uint8_t* blosc2_flags = *chunk + BLOSC2_CHUNK_BLOSC2_FLAGS;
    *blosc2_flags |= 0x08U;

    // Add the trailer (currently, nchunk + offset + block_csizes)
    if (frame->sframe) {
      *(int32_t*)(*chunk + trailer_offset) = offset;
      *(int64_t*)(*chunk + trailer_offset + sizeof(int32_t)) = offset;
    }
    else {
      *(int32_t*)(*chunk + trailer_offset) = nchunk;
      *(int64_t*)(*chunk + trailer_offset + sizeof(int32_t)) = header_len + offset;
    }

    int32_t* block_csizes = malloc(nblocks * sizeof(int32_t));

    int memcpyed = *(*chunk + BLOSC2_CHUNK_FLAGS) & (uint8_t)BLOSC_MEMCPYED;
    if (memcpyed) {
      // When memcpyed the blocksizes are trivial to compute
      for (int i = 0; i < (int)nblocks; i++) {
        block_csizes[i] = (int)chunk_blocksize;
      }
    }
    else {
      // In regular, compressed chunks, we need to sort the bstarts (they can be out
      // of order because of multi-threading), and get a reverse index too.
      memcpy(block_csizes, *chunk + BLOSC_EXTENDED_HEADER_LENGTH, nblocks * sizeof(int32_t));
      // Helper structure to keep track of original indexes
      struct csize_idx *csize_idx = malloc(nblocks * sizeof(struct csize_idx));
      for (int n = 0; n < (int)nblocks; n++) {
        csize_idx[n].val = block_csizes[n];
        csize_idx[n].idx = n;
      }
      qsort(csize_idx, nblocks, sizeof(struct csize_idx), &sort_offset);
      // Compute the actual csizes
      int idx;
      for (int n = 0; n < (int)nblocks - 1; n++) {
        idx = csize_idx[n].idx;
        block_csizes[idx] = csize_idx[n + 1].val - csize_idx[n].val;
      }
      idx = csize_idx[nblocks - 1].idx;
      block_csizes[idx] = (int)chunk_cbytes - csize_idx[nblocks - 1].val;
      free(csize_idx);
    }
    // Copy the csizes at the end of the trailer
    void *trailer_csizes = *chunk + lazy_chunk_bytes - nblocks * sizeof(int32_t);
    memcpy(trailer_csizes, block_csizes, nblocks * sizeof(int32_t));
    free(block_csizes);
  } else {
    // The chunk is in memory and just one pointer away
    *chunk = frame->cframe + header_len + offset;
    if ((int64_t)header_len + offset + BLOSC_MIN_HEADER_LENGTH > frame->len) {
      BLOSC_TRACE_ERROR("Cannot read the header for chunk in the (contiguous) frame.");
      rc = BLOSC2_ERROR_READ_BUFFER;
    } else {
      rc = blosc2_cbuffer_sizes(*chunk, NULL, &lazy_chunk_bytes, NULL);
    }
  }

  // Close the file pointer if it is open.
  // If there was an error, free the allocated chunk if necessary and return the error code.
  // If successful, return the size of the lazy chunk.
  // <MASK>
}