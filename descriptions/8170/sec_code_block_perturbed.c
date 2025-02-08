if ((image != (Image *) NULL) && (image->columns != 0) && (image->rows != 0))
    property_image=image;
  else
    {
      property_image=AcquireImage(image_info,exception);
      (void) SetImageExtent(property_image,1,1,exception);
      (void) SetImageBackgroundColor(property_image,exception);
    }
  interpret_text=AcquireString(embed_text); /* new string with extra space */
  extent=MagickPathExtent;