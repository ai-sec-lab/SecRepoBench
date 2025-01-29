size_t bits_dummy;
  JxlDecoderStatus status = PrepareSizeCheck(dec, format, &bits_dummy);
  if (status != JXL_DEC_SUCCESS) return status;

  dec->image_out_buffer_set = true;