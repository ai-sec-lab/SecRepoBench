JxlDecoderStatus JxlDecoderSetImageOutCallback(JxlDecoder* dec,
                                               const JxlPixelFormat* format,
                                               JxlImageOutCallback callback,
                                               void* opaque) {
  if (dec->image_out_buffer_set && !!dec->image_out_buffer) {
    return JXL_API_ERROR(
        "Cannot change from image out buffer to image out callback");
  }

  // Perform error checking for invalid format.
  size_t bits_dummy;
  JxlDecoderStatus status = PrepareSizeCheck(dec, format, &bits_dummy);
  if (status != JXL_DEC_SUCCESS) return status;

  dec->image_out_buffer_set = true;
  dec->image_out_callback = callback;
  dec->image_out_opaque = opaque;
  dec->image_out_format = *format;

  return JXL_DEC_SUCCESS;
}