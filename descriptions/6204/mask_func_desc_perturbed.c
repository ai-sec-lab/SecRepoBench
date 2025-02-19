MagickExport MagickBooleanType QueryColorCompliance(const char *name,
  const ComplianceType compliance,PixelInfo *pixelinfo,ExceptionInfo *exception)
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
  assert(pixelinfo != (PixelInfo *) NULL);
  GetPixelInfo((Image *) NULL,pixelinfo);
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
      pixelinfo->colorspace=sRGBColorspace;
      pixelinfo->depth=depth;
      pixelinfo->alpha_trait=UndefinedPixelTrait;
      range=GetQuantumRange(depth);
      pixelinfo->red=(double) ScaleAnyToQuantum(pixel.red,range);
      pixelinfo->green=(double) ScaleAnyToQuantum(pixel.green,range);
      pixelinfo->blue=(double) ScaleAnyToQuantum(pixel.blue,range);
      pixelinfo->alpha=(double) OpaqueAlpha;
      if ((n % 3) != 0)
        {
          pixelinfo->alpha_trait=BlendPixelTrait;
          pixelinfo->alpha=(double) ScaleAnyToQuantum(pixel.alpha,range);
        }
      pixelinfo->black=0.0;
      return(MagickTrue);
    }
  if (strchr(name,'(') != (char *) NULL)
    {
      // Declare a character array `colorspace` for storing the colorspace name,  
      // ensuring it is large enough to hold typical color representations.  
      // Introduce a `MagickBooleanType` variable `icc_color` to track if the color  
      // is in an ICC color space. This section of code prepares for parsing colors  
      // described in a string format, such as "rgb(100,255,0)".
      // <MASK>
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
      pixelinfo->alpha_trait=UndefinedPixelTrait;
      if ((i > 0) && (colorspace[i] == 'a'))
        {
          colorspace[i]='\0';
          pixelinfo->alpha_trait=BlendPixelTrait;
        }
      type=ParseCommandOption(MagickColorspaceOptions,MagickFalse,colorspace);
      if (type < 0)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),
            OptionWarning,"UnrecognizedColor","`%s'",name);
          return(MagickFalse);
        }
      pixelinfo->colorspace=(ColorspaceType) type;
      if ((icc_color == MagickFalse) && (pixelinfo->colorspace == RGBColorspace))
        {
          pixelinfo->colorspace=sRGBColorspace;  /* as required by SVG standard */
          pixelinfo->depth=8;
        }
      SetGeometryInfo(&geometry_info);
      flags=ParseGeometry(name+i+1,&geometry_info);
      if (flags == 0)
        {
          char
            *colorname;

          ColorspaceType
            colorspaceType;

          colorspaceType=pixelinfo->colorspace;
          colorname=AcquireString(name+i+1);
          (void) SubstituteString(&colorname,")","");
          (void) QueryColorCompliance(colorname,AllCompliance,pixelinfo,exception);
          colorname=DestroyString(colorname);
          pixelinfo->colorspace=colorspaceType;
        }
      else
        {
          if ((flags & PercentValue) != 0)
            scale=(double) (QuantumRange/100.0);
          if ((flags & RhoValue) != 0)
            pixelinfo->red=(double) ClampToQuantum((MagickRealType) (scale*
              geometry_info.rho));
          if ((flags & SigmaValue) != 0)
            pixelinfo->green=(double) ClampToQuantum((MagickRealType) (scale*
              geometry_info.sigma));
          if ((flags & XiValue) != 0)
            pixelinfo->blue=(double) ClampToQuantum((MagickRealType) (scale*
              geometry_info.xi));
          pixelinfo->alpha=(double) OpaqueAlpha;
          if ((flags & PsiValue) != 0)
            {
              if (pixelinfo->colorspace == CMYKColorspace)
                pixelinfo->black=(double) ClampToQuantum((MagickRealType) (
                  scale*geometry_info.psi));
              else
                if (pixelinfo->alpha_trait != UndefinedPixelTrait)
                  pixelinfo->alpha=(double) ClampToQuantum(QuantumRange*
                    geometry_info.psi);
            }
          if (((flags & ChiValue) != 0) &&
              (pixelinfo->alpha_trait != UndefinedPixelTrait))
            pixelinfo->alpha=(double) ClampToQuantum(QuantumRange*
              geometry_info.chi);
          if (pixelinfo->colorspace == LabColorspace)
            {
              if ((flags & SigmaValue) != 0)
                pixelinfo->green=(MagickRealType) ClampToQuantum((MagickRealType)
                  (scale*geometry_info.sigma+(QuantumRange+1)/2.0));
              if ((flags & XiValue) != 0)
                pixelinfo->blue=(MagickRealType) ClampToQuantum((MagickRealType)
                  (scale*geometry_info.xi+(QuantumRange+1)/2.0));
            }
          if (LocaleCompare(colorspace,"gray") == 0)
            {
              pixelinfo->green=pixelinfo->red;
              pixelinfo->blue=pixelinfo->red;
              if (((flags & SigmaValue) != 0) &&
                  (pixelinfo->alpha_trait != UndefinedPixelTrait))
                pixelinfo->alpha=(double) ClampToQuantum(QuantumRange*
                  geometry_info.sigma);
              if ((icc_color == MagickFalse) &&
                  (pixelinfo->colorspace == LinearGRAYColorspace))
                {
                  pixelinfo->colorspace=GRAYColorspace;
                  pixelinfo->depth=8;
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
                pixelinfo->colorspace=HCLColorspace;
              else
                if (LocaleCompare(colorspace,"HSB") == 0)
                  pixelinfo->colorspace=HSBColorspace;
                else
                  if (LocaleCompare(colorspace,"HSL") == 0)
                    pixelinfo->colorspace=HSLColorspace;
                  else
                    if (LocaleCompare(colorspace,"HWB") == 0)
                      pixelinfo->colorspace=HWBColorspace;
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
              pixelinfo->colorspace=sRGBColorspace;
              pixelinfo->red=(MagickRealType) red;
              pixelinfo->green=(MagickRealType) green;
              pixelinfo->blue=(MagickRealType) blue;
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
  pixelinfo->colorspace=sRGBColorspace;
  if ((LocaleNCompare(name,"gray",4) == 0) || 
      (LocaleNCompare(name,"grey",4) == 0))
    pixelinfo->colorspace=GRAYColorspace;
  pixelinfo->depth=8;
  pixelinfo->alpha_trait=p->color.alpha != OpaqueAlpha ? BlendPixelTrait :
    UndefinedPixelTrait;
  pixelinfo->red=(double) p->color.red;
  pixelinfo->green=(double) p->color.green;
  pixelinfo->blue=(double) p->color.blue;
  pixelinfo->alpha=(double) p->color.alpha;
  pixelinfo->black=0.0;
  return(MagickTrue);
}