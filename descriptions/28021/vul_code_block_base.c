if ((image_info->ping == MagickFalse) && (jng_color_type >= 12))
    {
      if (jng_alpha_compression_method == 0)
        {
          png_byte
            data[5];
          (void) WriteBlobMSBULong(alpha_image,0x00000000L);
          PNGType(data,mng_IEND);
          LogPNGChunk(logging,mng_IEND,0L);
          (void) WriteBlob(alpha_image,4,data);
          (void) WriteBlobMSBULong(alpha_image,crc32(0,data,4));
        }

      (void) CloseBlob(alpha_image);

      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "    Reading alpha from alpha_blob.");

      (void) FormatLocaleString(alpha_image_info->filename,MagickPathExtent,
        "%s",alpha_image->filename);

      jng_image=ReadImage(alpha_image_info,exception);

      if (jng_image != (Image *) NULL)
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          s=GetVirtualPixels(jng_image,0,y,image->columns,1,exception);
          q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
          if ((s == (const Quantum *)  NULL) || (q == (Quantum *) NULL))
            break;

          if (image->alpha_trait != UndefinedPixelTrait)
            for (x=(ssize_t) image->columns; x != 0; x--)
            {
              SetPixelAlpha(image,GetPixelRed(jng_image,s),q);
              q+=GetPixelChannels(image);
              s+=GetPixelChannels(jng_image);
            }

          else
            for (x=(ssize_t) image->columns; x != 0; x--)
            {
              SetPixelAlpha(image,GetPixelRed(jng_image,s),q);
              if (GetPixelAlpha(image,q) != OpaqueAlpha)
                image->alpha_trait=BlendPixelTrait;
              q+=GetPixelChannels(image);
              s+=GetPixelChannels(jng_image);
            }

          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
        }
      (void) RelinquishUniqueFileResource(alpha_image->filename);
      alpha_image=DestroyImageList(alpha_image);
      alpha_image_info=DestroyImageInfo(alpha_image_info);
      if (jng_image != (Image *) NULL)
        jng_image=DestroyImage(jng_image);
    }

  /* Read the JNG image.  */

  if (mng_info->mng_type == 0)
    {
      mng_info->mng_width=jng_width;
      mng_info->mng_height=jng_height;
    }

  if (image->page.width == 0 && image->page.height == 0)
    {
      image->page.width=jng_width;
      image->page.height=jng_height;
    }

  if (image->page.x == 0 && image->page.y == 0)
    {
      image->page.x=mng_info->x_off[mng_info->object_id];
      image->page.y=mng_info->y_off[mng_info->object_id];
    }

  else
    {
      image->page.y=mng_info->y_off[mng_info->object_id];
    }

  mng_info->image_found++;
  status=SetImageProgress(image,LoadImagesTag,2*TellBlob(image),
    2*GetBlobSize(image));

  if (status == MagickFalse)
    return(DestroyImageList(image));

  if (logging != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "  exit ReadOneJNGImage()");

  return(image);