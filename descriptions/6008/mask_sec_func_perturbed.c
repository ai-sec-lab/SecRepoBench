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
    channelindex;

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
  for (channelindex=0; channelindex < (ssize_t) psd_info->channels; channelindex++)
  {
    // <MASK>

    if (status != MagickFalse)
      status=SetImageProgress(image,LoadImagesTag,channelindex,psd_info->channels);

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