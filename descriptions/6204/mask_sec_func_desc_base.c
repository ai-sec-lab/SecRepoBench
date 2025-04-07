MagickExport MagickBooleanType QueryColorCompliance(const char *name,
  const ComplianceType compliance,PixelInfo *color,ExceptionInfo *exception)
{
  extern const char
    BackgroundColor[];

  GeometryInfo
    geometry_info;

  double
    scale;

  MagickStatusType
    flags;

  register const ColorInfo
    *p;

  register ssize_t
    i;

  ssize_t
    type;

  /*
    Initialize color return value.
  */
  assert(name != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",name);
  assert(color != (PixelInfo *) NULL);
  GetPixelInfo((Image *) NULL,color);
  if ((name == (char *) NULL) || (*name == '\0'))
    name=BackgroundColor;
  while (isspace((int) ((unsigned char) *name)) != 0)
    name++;
  if (*name == '#')
    {
      char
        c;

      PixelPacket
        pixel;

      QuantumAny
        range;

      size_t
        depth,
        n;

      /*
        Parse hex color.
      */
      (void) ResetMagickMemory(&pixel,0,sizeof(pixel));
      name++;
      for (n=0; isxdigit((int) ((unsigned char) name[n])) != 0; n++) ;
      if ((n % 3) == 0)
        {
          do
          {
            pixel.red=pixel.green;
            pixel.green=pixel.blue;
            pixel.blue=0;
            for (i=(ssize_t) (n/3-1); i >= 0; i--)
            {
              c=(*name++);
              pixel.blue<<=4;
              if ((c >= '0') && (c <= '9'))
                pixel.blue|=(int) (c-'0');
              else
                if ((c >= 'A') && (c <= 'F'))
                  pixel.blue|=(int) c-((int) 'A'-10);
                else
                  if ((c >= 'a') && (c <= 'f'))
                    pixel.blue|=(int) c-((int) 'a'-10);
                  else
                    return(MagickFalse);
            }
          } while (isxdigit((int) ((unsigned char) *name)) != 0);
          depth=4*(n/3);
        }
      else
        {
          if ((n % 4) != 0)
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionWarning,"UnrecognizedColor","`%s'",name);
              return(MagickFalse);
            }
          do
          {
            pixel.red=pixel.green;
            pixel.green=pixel.blue;
            pixel.blue=pixel.alpha;
            pixel.alpha=0;
            for (i=(ssize_t) (n/4-1); i >= 0; i--)
            {
              c=(*name++);
              pixel.alpha<<=4;
              if ((c >= '0') && (c <= '9'))
                pixel.alpha|=(int) (c-'0');
              else
                if ((c >= 'A') && (c <= 'F'))
                  pixel.alpha|=(int) c-((int) 'A'-10);
                else
                  if ((c >= 'a') && (c <= 'f'))
                    pixel.alpha|=(int) c-((int) 'a'-10);
                  else
                    return(MagickFalse);
            }
          } while (isxdigit((int) ((unsigned char) *name)) != 0);
          depth=4*(n/4);
        }
      color->colorspace=sRGBColorspace;
      color->depth=depth;
      color->alpha_trait=UndefinedPixelTrait;
      range=GetQuantumRange(depth);
      color->red=(double) ScaleAnyToQuantum(pixel.red,range);
      color->green=(double) ScaleAnyToQuantum(pixel.green,range);
      color->blue=(double) ScaleAnyToQuantum(pixel.blue,range);
      color->alpha=(double) OpaqueAlpha;
      if ((n % 3) != 0)
        {
          color->alpha_trait=BlendPixelTrait;
          color->alpha=(double) ScaleAnyToQuantum(pixel.alpha,range);
        }
      color->black=0.0;
      return(MagickTrue);
    }
  if (strchr(name,'(') != (char *) NULL)
    {
      // Initialize a character array to store the colorspace name from the input string.
      // Define a boolean variable to indicate if the color is an ICC profile color.
      // Copy the input color name into the colorspace array for parsing.
      // Identify and isolate the colorspace name by finding the first occurrence of an opening parenthesis and terminate the string there.
      // <MASK>
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
  /*
    Parse named color.
  */
  p=GetColorCompliance(name,compliance,exception);
  if (p == (const ColorInfo *) NULL)
    return(MagickFalse);
  color->colorspace=sRGBColorspace;
  if ((LocaleNCompare(name,"gray",4) == 0) || 
      (LocaleNCompare(name,"grey",4) == 0))
    color->colorspace=GRAYColorspace;
  color->depth=8;
  color->alpha_trait=p->color.alpha != OpaqueAlpha ? BlendPixelTrait :
    UndefinedPixelTrait;
  color->red=(double) p->color.red;
  color->green=(double) p->color.green;
  color->blue=(double) p->color.blue;
  color->alpha=(double) p->color.alpha;
  color->black=0.0;
  return(MagickTrue);
}