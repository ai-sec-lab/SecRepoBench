MagickExport MagickBooleanType DrawImage(Image *image,const DrawInfo *draw_info,
  ExceptionInfo *exception)
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
    factor,
    points_extent,
    primitive_extent;

  DrawInfo
    **graphic_context;

  MagickBooleanType
    proceed;

  MagickSizeType
    number_points;

  MagickStatusType
    status;

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
    number_stops;

  ssize_t
    j,
    k,
    n;

  StopInfo
    *stops;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(draw_info != (DrawInfo *) NULL);
  assert(draw_info->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if ((draw_info->primitive == (char *) NULL) ||
      (*draw_info->primitive == '\0'))
    return(MagickFalse);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DrawEvent,GetMagickModule(),"begin draw-image");
  primitive=(char *) NULL;
  if (*draw_info->primitive != '@')
    primitive=AcquireString(draw_info->primitive);
  else
    if (*(draw_info->primitive+1) != '-')
      primitive=FileToString(draw_info->primitive+1,~0UL,exception);
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
  // <MASK>
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DrawEvent,GetMagickModule(),"end draw-image");
  /*
    Relinquish resources.
  */
  token=DestroyString(token);
  if (primitive_info != (PrimitiveInfo *) NULL)
    primitive_info=(PrimitiveInfo *) RelinquishMagickMemory(primitive_info);
  primitive=DestroyString(primitive);
  for ( ; n >= 0; n--)
    graphic_context[n]=DestroyDrawInfo(graphic_context[n]);
  graphic_context=(DrawInfo **) RelinquishMagickMemory(graphic_context);
  if (status == MagickFalse)
    ThrowBinaryException(DrawError,"NonconformingDrawingPrimitiveDefinition",
      keyword);
  return(status != 0 ? MagickTrue : MagickFalse);
}