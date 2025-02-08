static MagickBooleanType ReadHEICImageByID(const ImageInfo *image_info,
  Image *image,struct heif_context *heif_context,heif_item_id image_id,
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
  if (IsHeifSuccess(&error,image,exception) == MagickFalse)
    return(MagickFalse);
  if (ReadHEICColorProfile(image,image_handle,exception) == MagickFalse)
    {
      heif_image_handle_release(image_handle);
      return(MagickFalse);
    }
  if (ReadHEICExifProfile(image,image_handle,exception) == MagickFalse)
    {
      heif_image_handle_release(image_handle);
      return(MagickFalse);
    }
  /*
    Set image size.
  */
  image->depth=8;
  image->columns=(size_t) heif_image_handle_get_width(image_handle);
  image->rows=(size_t) heif_image_handle_get_height(image_handle);
  if (image_info->ping != MagickFalse)
    {
      image->colorspace=YCbCrColorspace;
      heif_image_handle_release(image_handle);
      return(MagickTrue);
    }
  if (HEICSkipImage(image_info,image) != MagickFalse)
    {
      heif_image_handle_release(image_handle);
      return(MagickTrue);
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    {
      heif_image_handle_release(image_handle);
      return(MagickFalse);
    }
  /*
    Copy HEIF image into ImageMagick data structures.
  */
  (void) SetImageColorspace(image,YCbCrColorspace,exception);
  decode_options=(struct heif_decoding_options *) NULL;
  option=GetImageOption(image_info,"heic:preserve-orientation");
  if (IsStringTrue(option) == MagickTrue)
    {
      decode_options=heif_decoding_options_alloc();
      decode_options->ignore_transformations=1;
    }
  else
    (void) SetImageProperty(image,"exif:Orientation","1",exception);
  error=heif_decode_image(image_handle,&heif_image,heif_colorspace_YCbCr,
    heif_chroma_420,decode_options);
  // <MASK>
}