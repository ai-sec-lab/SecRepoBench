static MagickBooleanType WriteSingleWEBPPicture(const ImageInfo *image_info,
  Image *image,const WebPConfig *configure,WebPPicture *webppicture,
  MemoryInfo **memory_info,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  uint32_t
    *magick_restrict q;

  ssize_t
    y;

#if WEBP_ENCODER_ABI_VERSION >= 0x0100
  webppicture->progress_hook=WebPEncodeProgress;
  webppicture->user_data=(void *) image;
#endif
  webppicture->width=(int) image->columns;
  webppicture->height=(int) image->rows;
  webppicture->argb_stride=(int) image->columns;
  webppicture->use_argb=1;

  /*
    Allocate memory for pixels.
  */
  (void) TransformImageColorspace(image,sRGBColorspace,exception);
  *memory_info=AcquireVirtualMemory(image->columns,image->rows*
    sizeof(*(webppicture->argb)));

  if (*memory_info == (MemoryInfo *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  webppicture->argb=(uint32_t *) GetVirtualMemoryBlob(*memory_info);
  /*
    Convert image to WebP raster pixels.
  */
  status=MagickFalse;
  q=webppicture->argb;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    ssize_t
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      *q++=(uint32_t) (image->alpha_trait != UndefinedPixelTrait ? (uint32_t)
        ScaleQuantumToChar(GetPixelAlpha(image,p)) << 24 : 0xff000000) |
        ((uint32_t) ScaleQuantumToChar(GetPixelRed(image,p)) << 16) |
        ((uint32_t) ScaleQuantumToChar(GetPixelGreen(image,p)) << 8) |
        ((uint32_t) ScaleQuantumToChar(GetPixelBlue(image,p)));
      p+=GetPixelChannels(image);
    }
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }

  if (status != MagickFalse)
    status=(MagickBooleanType) WebPEncode(configure,webppicture);

  if (status == MagickFalse)
    (void) ThrowMagickException(exception,GetMagickModule(),CorruptImageError,
      WebPErrorCodeMessage(webppicture->error_code),"`%s'",image->filename);

  return(status);
}