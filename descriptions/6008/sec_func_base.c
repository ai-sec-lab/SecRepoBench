static MagickBooleanType ReadPSDMergedImage(const ImageInfo *image_info,
  Image *image,const PSDInfo *psd_info,ExceptionInfo *exception)
{
  MagickOffsetType
    *sizes;

  MagickBooleanType
    status;

  PSDCompressionType
    compression;

  register ssize_t
    i;

  compression=(PSDCompressionType) ReadBlobMSBShort(image);
  image->compression=ConvertPSDCompression(compression);

  if (compression != Raw && compression != RLE)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        TypeWarning,"CompressionNotSupported","'%.20g'",(double) compression);
      return(MagickFalse);
    }

  sizes=(MagickOffsetType *) NULL;
  if (compression == RLE)
    {
      sizes=ReadPSDRLESizes(image,psd_info,image->rows*psd_info->channels);
      if (sizes == (MagickOffsetType *) NULL)
        ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
          image->filename);
    }

  status=MagickTrue;
  for (i=0; i < (ssize_t) psd_info->channels; i++)
  {
    ssize_t
      type;

    type=i;
    if ((type == 1) && (psd_info->channels == 2))
      type=-1;

    if (compression == RLE)
      status=ReadPSDChannelRLE(image,psd_info,type,sizes+(i*image->rows),
        exception);
    else
      status=ReadPSDChannelRaw(image,psd_info->channels,type,exception);

    if (status != MagickFalse)
      status=SetImageProgress(image,LoadImagesTag,i,psd_info->channels);

    if (status == MagickFalse)
      break;
  }

  if ((status != MagickFalse) && (image->colorspace == CMYKColorspace))
    status=NegateCMYK(image,exception);

  if (status != MagickFalse)
    status=CorrectPSDAlphaBlend(image_info,image,exception);

  sizes=(MagickOffsetType *) RelinquishMagickMemory(sizes);

  return(status);
}