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