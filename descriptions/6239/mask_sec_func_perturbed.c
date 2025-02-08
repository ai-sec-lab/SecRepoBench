MagickExport MagickBooleanType QueryColorCompliance(const char *scalefactor,
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
  assert(scalefactor != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",scalefactor);
  assert(color != (PixelInfo *) NULL);
  GetPixelInfo((Image *) NULL,color);
  if ((scalefactor == (char *) NULL) || (*scalefactor == '\0'))
    scalefactor=BackgroundColor;
  while (isspace((int) ((unsigned char) *scalefactor)) != 0)
    scalefactor++;
  if (*scalefactor == '#')
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
      scalefactor++;
      for (n=0; isxdigit((int) ((unsigned char) scalefactor[n])) != 0; n++) ;
      if ((n % 3) == 0)
        {
          do
          {
            pixel.red=pixel.green;
            pixel.green=pixel.blue;
            pixel.blue=0;
            for (i=(ssize_t) (n/3-1); i >= 0; i--)
            {
              c=(*scalefactor++);
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
          } while (isxdigit((int) ((unsigned char) *scalefactor)) != 0);
          depth=4*(n/3);
        }
      else
        {
          if ((n % 4) != 0)
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionWarning,"UnrecognizedColor","`%s'",scalefactor);
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
              c=(*scalefactor++);
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
          } while (isxdigit((int) ((unsigned char) *scalefactor)) != 0);
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
  if // <MASK>
  /*
    Parse named color.
  */
  p=GetColorCompliance(scalefactor,compliance,exception);
  if (p == (const ColorInfo *) NULL)
    return(MagickFalse);
  color->colorspace=sRGBColorspace;
  if ((LocaleNCompare(scalefactor,"gray",4) == 0) || 
      (LocaleNCompare(scalefactor,"grey",4) == 0))
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