int _blosc_getitem(blosc2_context* context, blosc_header* hdr, const void* src, int32_t srcsize,
                   int start, int nitems, void* dest, int32_t destsize) {
  uint8_t* _src = (uint8_t*)(src);  /* current pos for source buffer */
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

  ebsize = hdr->blocksize + hdr->typesize * (signed)sizeof(int32_t);
  struct thread_context* scontext = context->serial_context;
  /* Resize the temporaries in serial context if needed */
  if (hdr->blocksize > scontext->tmp_blocksize) {
    my_free(scontext->tmp);
    scontext->tmp_nbytes = (size_t)4 * ebsize;
    scontext->tmp = my_malloc(scontext->tmp_nbytes);
    BLOSC_ERROR_NULL(scontext->tmp, BLOSC2_ERROR_MEMORY_ALLOC);
    scontext->tmp2 = scontext->tmp + ebsize;
    scontext->tmp3 = scontext->tmp2 + ebsize;
    scontext->tmp4 = scontext->tmp3 + ebsize;
    scontext->tmp_blocksize = (int32_t)hdr->blocksize;
  }

  for (j = 0; j < context->nblocks; j++) {
    bsize = hdr->blocksize;
    leftoverblock = 0;
    if ((j == context->nblocks - 1) && (context->leftover > 0)) {
      bsize = context->leftover;
      leftoverblock = 1;
    }

    /* Compute start & stop for each block */
    startb = start * hdr->typesize - j * hdr->blocksize;
    stopb = stop * hdr->typesize - j * hdr->blocksize;
    if (stopb <= 0) {
      // We can exit as soon as this block is beyond stop
      break;
    }
    if (startb >= hdr->blocksize) {
      continue;
    }
    if (startb < 0) {
      startb = 0;
    }
    if (stopb > hdr->blocksize) {
      stopb = hdr->blocksize;
    }
    bsize2 = stopb - startb;

    /* Do the actual data copy */
    // Regular decompression.  Put results in tmp2.
    // If the block is aligned and the worst case fits in destination, let's avoid a copy
    bool get_single_block = ((startb == 0) && (bsize == nitems * hdr->typesize));
    uint8_t* tmp2 = get_single_block ? dest : scontext->tmp2;

    // If memcpyed we don't have a bstarts section (because it is not needed)
    int32_t src_offset = memcpyed ?
      context->header_overhead + j * bsize : sw32_(context->bstarts + j);

    int cbytes = blosc_d(context->serial_context, bsize, leftoverblock, memcpyed,
                         src, srcsize, src_offset, j,
                         tmp2, 0, scontext->tmp, scontext->tmp3);
    if (cbytes < 0) {
      ntbytes = cbytes;
      break;
    }
    if (!get_single_block) {
      /* Copy to destination */
      memcpy((uint8_t *) dest + ntbytes, tmp2 + startb, (unsigned int) bsize2);
    }
    ntbytes += bsize2;
  }

  return ntbytes;
}