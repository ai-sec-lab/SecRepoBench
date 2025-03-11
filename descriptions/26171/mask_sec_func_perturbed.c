static int blosc_d(
    struct thread_context* thread_context, int32_t bsize,
    int32_t leftoverblock, const uint8_t* src, int32_t srcsize, int32_t src_offset,
    uint8_t* dest, int32_t destinationoffset, uint8_t* tmp, uint8_t* tmp2) {
  blosc2_context* context = thread_context->parent_context;
  uint8_t* filters = context->filters;
  uint8_t *tmp3 = thread_context->tmp4;
  int32_t compformat = (context->header_flags & 0xe0) >> 5;
  // <MASK> /* Closes j < nstreams */

  if (last_filter_index >= 0) {
    int errcode = pipeline_d(context, bsize, dest, destinationoffset, tmp, tmp2, tmp3,
                             last_filter_index);
    if (errcode < 0)
      return errcode;
  }

  /* Return the number of uncompressed bytes */
  return (int)ntbytes;
}