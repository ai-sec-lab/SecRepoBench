static MagickBooleanType ReadHEICImageByID(const ImageInfo *image_info,
  Image *img,struct heif_context *heif_context,heif_item_id image_id,
  ExceptionInfo *exception)
{
  const char
    *option;

  int
    stride_y,
    stride_cb,
    stride_cr;

  MagickBooleanType
    status;

  ssize_t
    y;

  struct heif_decoding_options
    *decode_options;

  struct heif_error
    error;

  struct heif_image
    *heif_image;

  struct heif_image_handle
    *image_handle;

  uint8_t
    *p_y,
    *p_cb,
    *p_cr;

  error=heif_context_get_image_handle(heif_context,image_id,&image_handle);
  if (IsHeifSuccess(&error,img,exception) == MagickFalse)
    return(MagickFalse);
  if (ReadHEICColorProfile(img,image_handle,exception) == MagickFalse)
    {
      heif_image_handle_release(image_handle);
      return(MagickFalse);
    }
  if (ReadHEICExifProfile(img,image_handle,exception) == MagickFalse)
    {
      heif_image_handle_release(image_handle);
      return(MagickFalse);
    }
  /*
    Set image size.
  */
  img->depth=8;
  img->columns=(size_t) heif_image_handle_get_width(image_handle);
  img->rows=(size_t) heif_image_handle_get_height(image_handle);
  if (image_info->ping != MagickFalse)
    {
      img->colorspace=YCbCrColorspace;
      heif_image_handle_release(image_handle);
      return(MagickTrue);
    }
  if (HEICSkipImage(image_info,img) != MagickFalse)
    {
      heif_image_handle_release(image_handle);
      return(MagickTrue);
    }
  status=SetImageExtent(img,img->columns,img->rows,exception);
  if (status == MagickFalse)
    {
      heif_image_handle_release(image_handle);
      return(MagickFalse);
    }
  /*
    Copy HEIF image into ImageMagick data structures.
  */
  (void) SetImageColorspace(img,YCbCrColorspace,exception);
  decode_options=(struct heif_decoding_options *) NULL;
  option=GetImageOption(image_info,"heic:preserve-orientation");
  if (IsStringTrue(option) == MagickTrue)
    {
      decode_options=heif_decoding_options_alloc();
      decode_options->ignore_transformations=1;
    }
  else
    (void) SetImageProperty(img,"exif:Orientation","1",exception);
  error=heif_decode_image(image_handle,&heif_image,heif_colorspace_YCbCr,
    heif_chroma_420,decode_options);
  if (IsHeifSuccess(&error,img,exception) == MagickFalse)
    {
      heif_image_handle_release(image_handle);
      return(MagickFalse);
    }
  if (decode_options != (struct heif_decoding_options *) NULL)
    {
      /*
        Correct the width and height of the image.
      */
      img->columns=(size_t) heif_image_get_width(heif_image,heif_channel_Y);
      img->rows=(size_t) heif_image_get_height(heif_image,heif_channel_Y);
      status=SetImageExtent(img,img->columns,img->rows,exception);
      heif_decoding_options_free(decode_options);
      if (status == MagickFalse)
        {
          heif_image_release(heif_image);
          heif_image_handle_release(image_handle);
          return(MagickFalse);
        }
    }
  p_y=heif_image_get_plane(heif_image,heif_channel_Y,&stride_y);
  p_cb=heif_image_get_plane(heif_image,heif_channel_Cb,&stride_cb);
  p_cr=heif_image_get_plane(heif_image,heif_channel_Cr,&stride_cr);
  for (y=0; y < (ssize_t) img->rows; y++)
  {
    Quantum
      *q;

    register ssize_t
      x;

    q=QueueAuthenticPixels(img,0,y,img->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) img->columns; x++)
    {
      SetPixelRed(img,ScaleCharToQuantum((unsigned char) p_y[y*
        stride_y+x]),q);
      SetPixelGreen(img,ScaleCharToQuantum((unsigned char) p_cb[(y/2)*
        stride_cb+x/2]),q);
      SetPixelBlue(img,ScaleCharToQuantum((unsigned char) p_cr[(y/2)*
        stride_cr+x/2]),q);
      q+=GetPixelChannels(img);
    }
    if (SyncAuthenticPixels(img,exception) == MagickFalse)
      break;
  }
  heif_image_release(heif_image);
  heif_image_handle_release(image_handle);
  return(MagickTrue);
}