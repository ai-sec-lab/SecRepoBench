if (decode_options != (struct heif_decoding_options *) NULL)
    {
      /*
        Correct the width and height of the image.
      */
      image->columns=(size_t) heif_image_get_width(heif_image,heif_channel_Y);
      image->rows=(size_t) heif_image_get_height(heif_image,heif_channel_Y);
      status=SetImageExtent(image,image->columns,image->rows,exception);
      heif_decoding_options_free(decode_options);
    }
  if ((IsHeifSuccess(&error,image,exception) == MagickFalse) ||
      (status == MagickFalse))
    {
      heif_image_release(heif_image);
      heif_image_handle_release(image_handle);
      return(MagickFalse);
    }
  p_y=heif_image_get_plane(heif_image,heif_channel_Y,&stride_y);
  p_cb=heif_image_get_plane(heif_image,heif_channel_Cb,&stride_cb);
  p_cr=heif_image_get_plane(heif_image,heif_channel_Cr,&stride_cr);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *q;

    register ssize_t
      x;

    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelRed(image,ScaleCharToQuantum((unsigned char) p_y[y*
        stride_y+x]),q);
      SetPixelGreen(image,ScaleCharToQuantum((unsigned char) p_cb[(y/2)*
        stride_cb+x/2]),q);
      SetPixelBlue(image,ScaleCharToQuantum((unsigned char) p_cr[(y/2)*
        stride_cr+x/2]),q);
      q+=GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
  heif_image_release(heif_image);
  heif_image_handle_release(image_handle);
  return(MagickTrue);