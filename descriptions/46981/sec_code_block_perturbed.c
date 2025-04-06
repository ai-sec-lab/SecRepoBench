if (image == (Image *) NULL)
      {
        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "exit ReadJNGImage() with error");
        return((Image *) NULL);
      }

    if (image->columns == 0 || image->rows == 0)
      {
        (void) CloseBlob(image);
        return(DestroyImageList(image));
      }

    mng_data->image=image;