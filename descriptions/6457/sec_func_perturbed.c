static MagickBooleanType WritePCDImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  Image
    *pcd_image;

  MagickBooleanType
    result;

  register ssize_t
    i;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  pcd_image=image;
  if (image->columns < image->rows)
    {
      Image
        *rotate_image;

      /*
        Rotate portrait to landscape.
      */
      rotate_image=RotateImage(image,90.0,exception);
      if (rotate_image == (Image *) NULL)
        return(MagickFalse);
      pcd_image=rotate_image;
      DestroyBlob(rotate_image);
      pcd_image->blob=ReferenceBlob(image->blob);
    }
  /*
    Open output image file.
  */
  result=OpenBlob(image_info,pcd_image,WriteBinaryBlobMode,exception);
  if (result == MagickFalse)
    {
      if (pcd_image != image)
        pcd_image=DestroyImage(pcd_image);
      return(result);
    }
  if (IssRGBCompatibleColorspace(pcd_image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(pcd_image,sRGBColorspace,exception);
  /*
    Write PCD image header.
  */
  for (i=0; i < 32; i++)
    (void) WriteBlobByte(pcd_image,0xff);
  for (i=0; i < 4; i++)
    (void) WriteBlobByte(pcd_image,0x0e);
  for (i=0; i < 8; i++)
    (void) WriteBlobByte(pcd_image,'\0');
  for (i=0; i < 4; i++)
    (void) WriteBlobByte(pcd_image,0x01);
  for (i=0; i < 4; i++)
    (void) WriteBlobByte(pcd_image,0x05);
  for (i=0; i < 8; i++)
    (void) WriteBlobByte(pcd_image,'\0');
  for (i=0; i < 4; i++)
    (void) WriteBlobByte(pcd_image,0x0A);
  for (i=0; i < 36; i++)
    (void) WriteBlobByte(pcd_image,'\0');
  for (i=0; i < 4; i++)
    (void) WriteBlobByte(pcd_image,0x01);
  for (i=0; i < 1944; i++)
    (void) WriteBlobByte(pcd_image,'\0');
  (void) WriteBlob(pcd_image,7,(const unsigned char *) "PCD_IPI");
  (void) WriteBlobByte(pcd_image,0x06);
  for (i=0; i < 1530; i++)
    (void) WriteBlobByte(pcd_image,'\0');
  if (image->columns < image->rows)
    (void) WriteBlobByte(pcd_image,'\1');
  else
    (void) WriteBlobByte(pcd_image,'\0');
  for (i=0; i < (3*0x800-1539); i++)
    (void) WriteBlobByte(pcd_image,'\0');
  /*
    Write PCD tiles.
  */
  result=WritePCDTile(pcd_image,"768x512>",192,128,exception);
  result=WritePCDTile(pcd_image,"768x512>",384,256,exception);
  result=WritePCDTile(pcd_image,"768x512>",768,512,exception);
  (void) CloseBlob(pcd_image);
  if (pcd_image != image)
    pcd_image=DestroyImage(pcd_image);
  return(result);
}