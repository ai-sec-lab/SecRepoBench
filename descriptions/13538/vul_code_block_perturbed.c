mask=CloneImage(image,layer_info->mask.page.width,
        layer_info->mask.page.height,MagickFalse,errorinfo);
      if (mask != (Image *) NULL)
        {
          (void) SetImageType(mask,GrayscaleType,errorinfo);
          channel_image=mask;
        }