if (image == (Image *) NULL)
      {
        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "exit ReadJNGImage() with error");
        if (mng_info->image != (Image *) NULL)
          {
            CloseBlob(mng_info->image);
            mng_info->image=DestroyImageList(mng_info->image);
          }
        return((Image *) NULL);
      }

    if (image->columns == 0 || image->rows == 0)
      {
        (void) CloseBlob(image);
        return(DestroyImageList(image));
      }

    mng_info->image=image;