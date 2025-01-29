int32_t ntbytes = 0;              /* the number of uncompressed bytes */
  int32_t bsize, bsize2, ebsize, leftoverblock;
  int32_t cbytes;
  int32_t startb, stopb;
  int32_t stop = start + nitems;
  int j, rc;

  if (nitems == 0) {
    // We have nothing to do
    return 0;
  }
  if (nitems * header->typesize > destsize) {
    BLOSC_TRACE_ERROR("`nitems`*`typesize` out of dest bounds.");
    return BLOSC2_ERROR_WRITE_BUFFER;
  }

  context->bstarts = (int32_t*)(_src + context->header_overhead);

  /* Check region boundaries */
  if ((start < 0) || (start * header->typesize > header->nbytes)) {
    BLOSC_TRACE_ERROR("`start` out of bounds.");
    return BLOSC2_ERROR_INVALID_PARAM;
  }

  if ((stop < 0) || (stop * header->typesize > header->nbytes)) {
    BLOSC_TRACE_ERROR("`start`+`nitems` out of bounds.");
    return BLOSC2_ERROR_INVALID_PARAM;
  }

  if (!context->special_type &&
      (_src + srcsize < (uint8_t *)(context->bstarts + context->nblocks))) {
    BLOSC_TRACE_ERROR("`bstarts` out of bounds.");
    return BLOSC2_ERROR_READ_BUFFER;
  }

  bool memcpyed = header->flags & (uint8_t)BLOSC_MEMCPYED;
  if (context->special_type) {
    // Fake a runlen as if its a memcpyed chunk
    memcpyed = true;
  }
