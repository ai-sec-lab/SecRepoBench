static MagickBooleanType RenderMVGContent(Image *image,
  const DrawInfo *draw_info,const size_t depth,ExceptionInfo *exception)
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
  assert(draw_info != (DrawInfo *) NULL);
  assert(draw_info->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (depth > MagickMaxRecursionDepth)
    ThrowBinaryException(DrawError,"VectorGraphicsNestedTooDeeply",
      image->filename);
  if ((draw_info->primitive == (char *) NULL) ||
      (*draw_info->primitive == '\0'))
    return(MagickFalse);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DrawEvent,GetMagickModule(),"begin draw-image");
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  // <MASK>
}