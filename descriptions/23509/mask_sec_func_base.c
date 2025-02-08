static MagickBooleanType RenderType(Image *image,const DrawInfo *draw_info,
  const PointInfo *offset,TypeMetric *metrics,ExceptionInfo *exception)
{
  const TypeInfo
    *type_info;

  DrawInfo
    *annotate_info;

  MagickBooleanType
    status;

  type_info=(const TypeInfo *) NULL;
  if (draw_info->font != (char *) NULL)
    {
      if (*draw_info->font == '@')
        {
          status=RenderFreetype(image,draw_info,draw_info->encoding,offset,
            metrics,exception);
          return(status);
        }
      if (*draw_info->font == '-')
        return(RenderX11(image,draw_info,offset,metrics,exception));
      if (*draw_info->font == '^')
        return(RenderPostscript(image,draw_info,offset,metrics,exception));
      if (IsPathAccessible(draw_info->font) != MagickFalse)
        {
          status=RenderFreetype(image,draw_info,draw_info->encoding,offset,
            metrics,exception);
          return(status);
        }
      type_info=GetTypeInfo(draw_info->font,exception);
      if (type_info == (const TypeInfo *) NULL)
        (void) ThrowMagickException(exception,GetMagickModule(),TypeWarning,
          "UnableToReadFont","`%s'",draw_info->font);
    }
  if ((type_info == (const TypeInfo *) NULL) &&
      (draw_info->family != (const char *) NULL))
    {
      // <MASK>
    }
  if (type_info == (const TypeInfo *) NULL)
    type_info=GetTypeInfoByFamily("Arial",draw_info->style,
      draw_info->stretch,draw_info->weight,exception);
  if (type_info == (const TypeInfo *) NULL)
    type_info=GetTypeInfoByFamily("Helvetica",draw_info->style,
      draw_info->stretch,draw_info->weight,exception);
  if (type_info == (const TypeInfo *) NULL)
    type_info=GetTypeInfoByFamily("Century Schoolbook",draw_info->style,
      draw_info->stretch,draw_info->weight,exception);
  if (type_info == (const TypeInfo *) NULL)
    type_info=GetTypeInfoByFamily("Sans",draw_info->style,
      draw_info->stretch,draw_info->weight,exception);
  if (type_info == (const TypeInfo *) NULL)
    type_info=GetTypeInfoByFamily((const char *) NULL,draw_info->style,
      draw_info->stretch,draw_info->weight,exception);
  if (type_info == (const TypeInfo *) NULL)
    type_info=GetTypeInfo("*",exception);
  if (type_info == (const TypeInfo *) NULL)
    {
      status=RenderFreetype(image,draw_info,draw_info->encoding,offset,metrics,
        exception);
      return(status);
    }
  annotate_info=CloneDrawInfo((ImageInfo *) NULL,draw_info);
  annotate_info->face=type_info->face;
  if (type_info->metrics != (char *) NULL)
    (void) CloneString(&annotate_info->metrics,type_info->metrics);
  if (type_info->glyphs != (char *) NULL)
    (void) CloneString(&annotate_info->font,type_info->glyphs);
  status=RenderFreetype(image,annotate_info,type_info->encoding,offset,metrics,
    exception);
  annotate_info=DestroyDrawInfo(annotate_info);
  return(status);
}