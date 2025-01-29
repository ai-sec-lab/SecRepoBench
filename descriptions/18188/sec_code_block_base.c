if (primitive_info->text == (char *) NULL)
        break;
      clone_info=AcquireImageInfo();
      composite_images=(Image *) NULL;
      if (LocaleNCompare(primitive_info->text,"data:",5) == 0)
        composite_images=ReadInlineImage(clone_info,primitive_info->text,
          exception);
      else
        {
          (void) CopyMagickString(clone_info->filename,primitive_info->text,
            MagickPathExtent);
          SetImageInfo(clone_info,0,exception);
          if (*clone_info->filename != '\0')
            composite_images=ReadImage(clone_info,exception);
        }