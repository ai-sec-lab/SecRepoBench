int _blosc_getitem(blosc2_context* context, blosc_header* header, const void* src, int32_t srcsize,
                   int start, int nitems, void* dest, int32_t destsize) {
  uint8_t* _src = (uint8_t*)(src);  /* current pos for source buffer */
  // Initialize variables for tracking uncompressed bytes, block sizes, and index positions.
  // Calculate the stopping position based on the start position and number of items.
  // Check if there are no items to process and return early if so.
  // Validate that the destination buffer is large enough for the decompressed data.
  // Set up block start positions based on the source buffer and context.
  // Verify that the start and stop positions are within valid bounds for the data.
  // Determine if the data should be treated as memcpyed based on header flags or context type.
  // Handle special cases for data that is memcpyed or has specific characteristics.
  // <MASK>

  ebsize = header->blocksize + header->typesize * (signed)sizeof(int32_t);
  struct thread_context* scontext = context->serial_context;
  /* Resize the temporaries in serial context if needed */
  if (header->blocksize > scontext->tmp_blocksize) {
    my_free(scontext->tmp);
    scontext->tmp_nbytes = (size_t)4 * ebsize;
    scontext->tmp = my_malloc(scontext->tmp_nbytes);
    BLOSC_ERROR_NULL(scontext->tmp, BLOSC2_ERROR_MEMORY_ALLOC);
    scontext->tmp2 = scontext->tmp + ebsize;
    scontext->tmp3 = scontext->tmp2 + ebsize;
    scontext->tmp4 = scontext->tmp3 + ebsize;
    scontext->tmp_blocksize = (int32_t)header->blocksize;
  }

  for (j = 0; j < context->nblocks; j++) {
    bsize = header->blocksize;
    leftoverblock = 0;
    if ((j == context->nblocks - 1) && (context->leftover > 0)) {
      bsize = context->leftover;
      leftoverblock = 1;
    }

    /* Compute start & stop for each block */
    startb = start * header->typesize - j * header->blocksize;
    stopb = stop * header->typesize - j * header->blocksize;
    if (stopb <= 0) {
      // We can exit as soon as this block is beyond stop
      break;
    }
    if (startb >= header->blocksize) {
      continue;
    }
    if (startb < 0) {
      startb = 0;
    }
    if (stopb > header->blocksize) {
      stopb = header->blocksize;
    }
    bsize2 = stopb - startb;

    /* Do the actual data copy */
    // Regular decompression.  Put results in tmp2.
    // If the block is aligned and the worst case fits in destination, let's avoid a copy
    bool get_single_block = ((startb == 0) && (bsize == nitems * header->typesize));
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