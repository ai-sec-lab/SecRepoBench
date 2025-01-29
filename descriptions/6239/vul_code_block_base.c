(strchr(name,'(') != (char *) NULL)
    {
      char
        colorspace[MagickPathExtent+1];

      MagickBooleanType
        icc_color;

      /*
        Parse color of the form rgb(100,255,0).
      */
      (void) memset(colorspace,0,sizeof(colorspace));
      (void) CopyMagickString(colorspace,name,MagickPathExtent);
      for (i=0; colorspace[i] != '\0'; i++)
        if (colorspace[i] == '(')
          break;
      colorspace[i--]='\0';
      scale=(double) ScaleCharToQuantum(1);
      icc_color=MagickFalse;
      if (LocaleNCompare(colorspace,"device-",7) == 0)
        {
          (void) CopyMagickString(colorspace,colorspace+7,MagickPathExtent);
          scale=(double) QuantumRange;
          icc_color=MagickTrue;
        }
      if (LocaleCompare(colorspace,"icc-color") == 0)
        {
          register ssize_t
            j;

          (void) CopyMagickString(colorspace,name+i+2,MagickPathExtent);
          for (j=0; colorspace[j] != '\0'; j++)
            if (colorspace[j] == ',')
              break;
          colorspace[j--]='\0';
          i+=j+3;
          scale=(double) QuantumRange;
          icc_color=MagickTrue;
        }
      LocaleLower(colorspace);
      color->alpha_trait=UndefinedPixelTrait;
      if ((i > 0) && (colorspace[i] == 'a'))
        {
          colorspace[i]='\0';
          color->alpha_trait=BlendPixelTrait;
        }
      type=ParseCommandOption(MagickColorspaceOptions,MagickFalse,colorspace);
      if (type < 0)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),
            OptionWarning,"UnrecognizedColor","`%s'",name);
          return(MagickFalse);
        }
      color->colorspace=(ColorspaceType) type;
      if ((icc_color == MagickFalse) && (color->colorspace == RGBColorspace))
        {
          color->colorspace=sRGBColorspace;  /* as required by SVG standard */
          color->depth=8;
        }
      SetGeometryInfo(&geometry_info);
      flags=ParseGeometry(name+i+1,&geometry_info);
      if (flags == 0)
        {
          char
            *colorname;

          ColorspaceType
            colorspaceType;

          colorspaceType=color->colorspace;
          colorname=AcquireString(name+i+1);
          (void) SubstituteString(&colorname,")","");
          (void) QueryColorCompliance(colorname,AllCompliance,color,exception);
          colorname=DestroyString(colorname);
          color->colorspace=colorspaceType;
        }
      else
        {
          if ((flags & PercentValue) != 0)
            scale=(double) (QuantumRange/100.0);
          if ((flags & RhoValue) != 0)
            color->red=(double) ClampToQuantum((MagickRealType) (scale*
              geometry_info.rho));
          if ((flags & SigmaValue) != 0)
            color->green=(double) ClampToQuantum((MagickRealType) (scale*
              geometry_info.sigma));
          if ((flags & XiValue) != 0)
            color->blue=(double) ClampToQuantum((MagickRealType) (scale*
              geometry_info.xi));
          color->alpha=(double) OpaqueAlpha;
          if ((flags & PsiValue) != 0)
            {
              if (color->colorspace == CMYKColorspace)
                color->black=(double) ClampToQuantum((MagickRealType) (
                  scale*geometry_info.psi));
              else
                if (color->alpha_trait != UndefinedPixelTrait)
                  color->alpha=(double) ClampToQuantum(QuantumRange*
                    geometry_info.psi);
            }
          if (((flags & ChiValue) != 0) &&
              (color->alpha_trait != UndefinedPixelTrait))
            color->alpha=(double) ClampToQuantum(QuantumRange*
              geometry_info.chi);
          if (color->colorspace == LabColorspace)
            {
              if ((flags & SigmaValue) != 0)
                color->green=(MagickRealType) ClampToQuantum((MagickRealType)
                  (scale*geometry_info.sigma+(QuantumRange+1)/2.0));
              if ((flags & XiValue) != 0)
                color->blue=(MagickRealType) ClampToQuantum((MagickRealType)
                  (scale*geometry_info.xi+(QuantumRange+1)/2.0));
            }
          if (LocaleCompare(colorspace,"gray") == 0)
            {
              color->green=color->red;
              color->blue=color->red;
              if (((flags & SigmaValue) != 0) &&
                  (color->alpha_trait != UndefinedPixelTrait))
                color->alpha=(double) ClampToQuantum(QuantumRange*
                  geometry_info.sigma);
              if ((icc_color == MagickFalse) &&
                  (color->colorspace == LinearGRAYColorspace))
                {
                  color->colorspace=GRAYColorspace;
                  color->depth=8;
                }
            }
          if ((LocaleCompare(colorspace,"HCL") == 0) ||
              (LocaleCompare(colorspace,"HSB") == 0) ||
              (LocaleCompare(colorspace,"HSL") == 0) ||
              (LocaleCompare(colorspace,"HWB") == 0))
            {
              double
                blue,
                green,
                red;

              if (LocaleCompare(colorspace,"HCL") == 0)
                color->colorspace=HCLColorspace;
              else
                if (LocaleCompare(colorspace,"HSB") == 0)
                  color->colorspace=HSBColorspace;
                else
                  if (LocaleCompare(colorspace,"HSL") == 0)
                    color->colorspace=HSLColorspace;
                  else
                    if (LocaleCompare(colorspace,"HWB") == 0)
                      color->colorspace=HWBColorspace;
              scale=1.0/255.0;
              if ((flags & PercentValue) != 0)
                scale=1.0/100.0;
              geometry_info.sigma*=scale;
              geometry_info.xi*=scale;
              if (LocaleCompare(colorspace,"HCL") == 0)
                ConvertHCLToRGB(fmod(fmod(geometry_info.rho,360.0)+360.0,
                  360.0)/360.0,geometry_info.sigma,geometry_info.xi,&red,
                  &green,&blue);
              else
                if (LocaleCompare(colorspace,"HSB") == 0)
                  ConvertHSBToRGB(fmod(fmod(geometry_info.rho,360.0)+360.0,
                    360.0)/360.0,geometry_info.sigma,geometry_info.xi,&red,
                    &green,&blue);
                else
                  if (LocaleCompare(colorspace,"HSL") == 0)
                    ConvertHSLToRGB(fmod(fmod(geometry_info.rho,360.0)+360.0,
                      360.0)/360.0,geometry_info.sigma,geometry_info.xi,&red,
                      &green,&blue);
                  else
                    ConvertHWBToRGB(fmod(fmod(geometry_info.rho,360.0)+360.0,
                      360.0)/360.0,geometry_info.sigma,geometry_info.xi,&red,
                      &green,&blue);
              color->colorspace=sRGBColorspace;
              color->red=(MagickRealType) red;
              color->green=(MagickRealType) green;
              color->blue=(MagickRealType) blue;
            }
        }
      return(MagickTrue);
    }