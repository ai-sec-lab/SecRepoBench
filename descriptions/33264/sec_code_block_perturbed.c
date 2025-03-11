uint8_t* _dest = (uint8_t*)(dest);
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
  if (nitems * hdr->typesize > destsize) {
    BLOSC_TRACE_ERROR("`nitems`*`typesize` out of dest bounds.");
    return BLOSC2_ERROR_WRITE_BUFFER;
  }

  context->bstarts = (int32_t*)(_src + context->header_overhead);

  /* Check region boundaries */
  if ((start < 0) || (start * hdr->typesize > hdr->nbytes)) {
    BLOSC_TRACE_ERROR("`start` out of bounds.");
    return BLOSC2_ERROR_INVALID_PARAM;
  }

  if ((stop < 0) || (stop * hdr->typesize > hdr->nbytes)) {
    BLOSC_TRACE_ERROR("`start`+`nitems` out of bounds.");
    return BLOSC2_ERROR_INVALID_PARAM;
  }

  if (!context->special_type &&
      (_src + srcsize < (uint8_t *)(context->bstarts + context->nblocks))) {
    BLOSC_TRACE_ERROR("`bstarts` out of bounds.");
    return BLOSC2_ERROR_READ_BUFFER;
  }

  bool memcpyed = hdr->flags & (uint8_t)BLOSC_MEMCPYED;
  if (context->special_type) {
    // Fake a runlen as if its a memcpyed chunk
    memcpyed = true;
  }

  bool is_lazy = ((context->header_overhead == BLOSC_EXTENDED_HEADER_LENGTH) &&
                  (context->blosc2_flags & 0x08u) && !context->special_type);
  if (memcpyed && !is_lazy && !context->postfilter) {
    // Short-circuit for (non-lazy) memcpyed or special values
    ntbytes = nitems * hdr->typesize;
    switch (context->special_type) {
      case BLOSC2_SPECIAL_VALUE:
        // All repeated values
        rc = set_values(context->typesize, _src, _dest, ntbytes);
        if (rc < 0) {
          BLOSC_TRACE_ERROR("set_values failed");
          return BLOSC2_ERROR_DATA;
        }
        break;
      case BLOSC2_SPECIAL_NAN:
        rc = set_nans(context->typesize, _dest, ntbytes);
        if (rc < 0) {
          BLOSC_TRACE_ERROR("set_nans failed");
          return BLOSC2_ERROR_DATA;
        }
        break;
      case BLOSC2_SPECIAL_ZERO:
        memset(_dest, 0, ntbytes);
        break;
      case BLOSC2_SPECIAL_UNINIT:
        // We do nothing here
        break;
      case BLOSC2_NO_SPECIAL:
        _src += context->header_overhead + start * context->typesize;
        memcpy(_dest, _src, ntbytes);
        break;
      default:
        BLOSC_TRACE_ERROR("Unhandled special value case");
        return -1;
    }
    return ntbytes;
  }