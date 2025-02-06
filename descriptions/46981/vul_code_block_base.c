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