size_t bits_dummy;
  JxlDecoderStatus status = PrepareSizeCheck(decoder, format, &bits_dummy);
  if (status != JXL_DEC_SUCCESS) return status;

  decoder->image_out_buffer_set = true;