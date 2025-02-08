if (status == MagickFalse)
    (void) ThrowMagickException(exception,GetMagickModule(),CorruptImageError,
      WebPErrorCodeMessage(webppicture->error_code),"`%s'",
      image->filename);

  return(status);