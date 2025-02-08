JxlDecoderStatus JxlDecoderSetImageOutCallback(JxlDecoder* decoder,
                                               const JxlPixelFormat* format,
                                               JxlImageOutCallback callback,
                                               void* opaque) {
  if (decoder->image_out_buffer_set && !!decoder->image_out_buffer) {
    return JXL_API_ERROR(
        "Cannot change from image out buffer to image out callback");
  }

  // Perform error checking for invalid format.
  size_t bits_dummy;
  JxlDecoderStatus status = PrepareSizeCheck(decoder, format, &bits_dummy);
  if (status != JXL_DEC_SUCCESS) return status;

  decoder->image_out_buffer_set = true;
  decoder->image_out_callback = callback;
  decoder->image_out_opaque = opaque;
  decoder->image_out_format = *format;

  return JXL_DEC_SUCCESS;
}