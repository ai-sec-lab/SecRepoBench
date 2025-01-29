char
        colorspace[MagickPathExtent+1];

      MagickBooleanType
        icc_color;

      /*
        Parse color of the form rgb(100,255,0).
      */
      (void) memset(colorspace,0,sizeof(colorspace));