mask=CloneImage(image,layer_info->mask.page.width,
        layer_info->mask.page.height,MagickFalse,exception);
      if (mask != (Image *) NULL)
        {
          (void) SetImageType(mask,GrayscaleType,exception);
          channel_image=mask;
        }