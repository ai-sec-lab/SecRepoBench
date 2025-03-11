int _blosc_getitem(blosc2_context* context, blosc_header* hdr, const void* src, int32_t srcsize,
                   int start, int nitems, void* dest, int32_t destsize) {
  uint8_t* _src = (uint8_t*)(src);  /* current pos for source buffer */
  // <MASK>

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