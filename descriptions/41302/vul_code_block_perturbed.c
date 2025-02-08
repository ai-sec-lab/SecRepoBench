while (next != (Image *) NULL)
      {
        if (PSDSkipImage(&psd_info,image_info,i++) == MagickFalse)
          {
            (void) SetImageProfile(next,GetStringInfoName(profile),profile,
              exception);
            if (replicate_profile == MagickFalse)
              break;
          }
        next=image->next;
      }
      profile=DestroyStringInfo(profile);