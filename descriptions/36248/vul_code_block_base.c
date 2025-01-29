if (status == MagickFalse)
    (void) ThrowMagickException(exception,GetMagickModule(),CorruptImageError,
      WebPErrorCodeMessage(picture->error_code),"`%s'",
      image->filename);

  return(status);