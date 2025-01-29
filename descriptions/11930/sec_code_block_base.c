assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  (void) ResetImagePixels(image,exception);
  (void) ResetImagePage(image,"0x0+0+0");
  /*
    Format caption.
  */