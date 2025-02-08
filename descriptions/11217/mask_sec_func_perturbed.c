static MagickBooleanType RenderMVGContent(Image *image,
  const DrawInfo *dinfo,const size_t depth,ExceptionInfo *exception)
{
#define RenderImageTag  "Render/Image"

  AffineMatrix
    affine,
    current;

  char
    keyword[MagickPathExtent],
    geometry[MagickPathExtent],
    *next_token,
    pattern[MagickPathExtent],
    *primitive,
    *token;

  const char
    *q;

  double
    angle,
    coordinates,
    cursor,
    factor,
    primitive_extent;

  DrawInfo
    *clone_info,
    **graphic_context;

  MagickBooleanType
    proceed;

  MagickStatusType
    status;

  MVGInfo
    mvg_info;

  PointInfo
    point;

  PrimitiveInfo
    *primitive_info;

  PrimitiveType
    primitive_type;

  register const char
    *p;

  register ssize_t
    i,
    x;

  SegmentInfo
    bounds;

  size_t
    extent,
    number_points,
    number_stops;

  SplayTreeInfo
    *macros;

  ssize_t
    defsDepth,
    j,
    k,
    n,
    symbolDepth;

  StopInfo
    *stops;

  TypeMetric
    metrics;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(dinfo != (DrawInfo *) NULL);
  assert(dinfo->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (depth > MagickMaxRecursionDepth)
    ThrowBinaryException(DrawError,"VectorGraphicsNestedTooDeeply",
      image->filename);
  if ((dinfo->primitive == (char *) NULL) ||
      (*dinfo->primitive == '\0'))
    return(MagickFalse);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DrawEvent,GetMagickModule(),"begin draw-image");
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  if (image->alpha_trait == UndefinedPixelTrait)
    {
      status=SetImageAlphaChannel(image,OpaqueAlphaChannel,exception);
      if (status == MagickFalse)
        return(status == 0 ? MagickFalse : MagickTrue);
    }
  primitive=(char *) NULL;
  if (*dinfo->primitive != '@')
    primitive=AcquireString(dinfo->primitive);
  else
    if ((strlen(dinfo->primitive) > 1) &&
        (*(dinfo->primitive+1) != '-'))
      primitive=FileToString(dinfo->primitive+1,~0UL,exception);
  if (primitive == (char *) NULL)
    return(MagickFalse);
  primitive_extent=(double) strlen(primitive);
  (void) SetImageArtifact(image,"MVG",primitive);
  n=0;
  number_stops=0;
  stops=(StopInfo *) NULL;
  /*
    Allocate primitive info memory.
  */
  graphic_context=(DrawInfo **) AcquireMagickMemory(sizeof(*graphic_context));
  if (graphic_context == (DrawInfo **) NULL)
    {
      primitive=DestroyString(primitive);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  number_points=PrimitiveExtentPad;
  primitive_info=(PrimitiveInfo *) AcquireQuantumMemory((size_t) number_points,
    sizeof(*primitive_info));
  if (primitive_info == (PrimitiveInfo *) NULL)
    {
      primitive=DestroyString(primitive);
      for ( ; n >= 0; n--)
        graphic_context[n]=DestroyDrawInfo(graphic_context[n]);
      graphic_context=(DrawInfo **) RelinquishMagickMemory(graphic_context);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  (void) memset(primitive_info,0,(size_t) number_points*
    sizeof(*primitive_info));
  // <MASK>
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DrawEvent,GetMagickModule(),"end draw-image");
  /*
    Relinquish resources.
  */
  macros=DestroySplayTree(macros);
  token=DestroyString(token);
  if (primitive_info != (PrimitiveInfo *) NULL)
    {
      for (i=0; primitive_info[i].primitive != UndefinedPrimitive; i++)
        if ((primitive_info[i].primitive == TextPrimitive) ||
            (primitive_info[i].primitive == ImagePrimitive))
          if (primitive_info[i].text != (char *) NULL)
            primitive_info[i].text=DestroyString(primitive_info[i].text);
      primitive_info=(PrimitiveInfo *) RelinquishMagickMemory(primitive_info);
    }
  primitive=DestroyString(primitive);
  if (stops != (StopInfo *) NULL)
    stops=(StopInfo *) RelinquishMagickMemory(stops);
  for ( ; n >= 0; n--)
    graphic_context[n]=DestroyDrawInfo(graphic_context[n]);
  graphic_context=(DrawInfo **) RelinquishMagickMemory(graphic_context);
  if (status == MagickFalse)
    ThrowBinaryException(DrawError,"NonconformingDrawingPrimitiveDefinition",
      keyword);
  return(status != 0 ? MagickTrue : MagickFalse);
}