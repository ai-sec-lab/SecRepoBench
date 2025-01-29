while (next != (Image *) NULL)
      {
        if (PSDSkipImage(&psd_info,image_info,i++) == MagickFalse)
          {
            (void) SetImageProfile(next,GetStringInfoName(profile),profile,
              exception);
            if (replicate_profile == MagickFalse)
              break;
          }
        next=next->next;
      }
      profile=DestroyStringInfo(profile);