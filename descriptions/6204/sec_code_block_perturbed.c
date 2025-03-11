char
        colorspace[MagickPathExtent+1];

      MagickBooleanType
        icc_color;

      /*
        Parse color of the form rgb(100,255,0).
      */
      (void) memset(colorspace,0,sizeof(colorspace));
      (void) CopyMagickString(colorspace,color_name,MagickPathExtent);
      for (i=0; colorspace[i] != '\0'; i++)
        if (colorspace[i] == '(')
          break;
      colorspace[i--]='\0';