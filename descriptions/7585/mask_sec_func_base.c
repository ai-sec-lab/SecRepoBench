static MagickBooleanType DrawDashPolygon(const DrawInfo *draw_info,
  const PrimitiveInfo *primitive_info,Image *image,ExceptionInfo *exception)
{
  DrawInfo
    *clone_info;

  double
    length,
    maximum_length,
    offset,
    scale,
    total_length;

  MagickStatusType
    status;

  PrimitiveInfo
    *dash_polygon;

  register ssize_t
    i;

  register double
    dx,
    dy;

  size_t
    number_vertices;

  ssize_t
    j,
    n;

  assert(draw_info != (const DrawInfo *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DrawEvent,GetMagickModule(),"    begin draw-dash");
  for (i=0; primitive_info[i].primitive != UndefinedPrimitive; i++) ;
  number_vertices=(size_t) i;
  dash_polygon=(PrimitiveInfo *) AcquireQuantumMemory((size_t)
    (2UL*number_vertices+32UL),sizeof(*dash_polygon));
  if (dash_polygon == (PrimitiveInfo *) NULL)
    return(MagickFalse);
  clone_info=CloneDrawInfo((ImageInfo *) NULL,draw_info);
  clone_info->miterlimit=0;
  dash_polygon[0]=primitive_info[0];
  scale=ExpandAffine(&draw_info->affine);
  length=scale*(draw_info->dash_pattern[0]-0.5);
  offset=fabs(draw_info->dash_offset) >= DrawEpsilon ?
    scale*draw_info->dash_offset : 0.0;
  j=1;
  for (n=0; offset > 0.0; j=0)
  {
    if (draw_info->dash_pattern[n] <= 0.0)
      break;
    length=scale*(draw_info->dash_pattern[n]+(n == 0 ? -0.5 : 0.5));
    if (offset > length)
      {
        offset-=length;
        n++;
        length=scale*(draw_info->dash_pattern[n]+0.5);
        continue;
      }
    if (offset < length)
      {
        length-=offset;
        offset=0.0;
        break;
      }
    offset=0.0;
    n++;
  }
  status=MagickTrue;
  maximum_length=0.0;
  total_length=0.0;
  for (i=1; (i < (ssize_t) number_vertices) && (length >= 0.0); i++)
  {
    dx=primitive_info[i].point.x-primitive_info[i-1].point.x;
    dy=primitive_info[i].point.y-primitive_info[i-1].point.y;
    maximum_length=hypot(dx,dy);
    // <MASK>
    length-=(maximum_length-total_length);
    if ((n & 0x01) != 0)
      continue;
    dash_polygon[j]=primitive_info[i];
    dash_polygon[j].coordinates=1;
    j++;
  }
  if ((total_length <= maximum_length) && ((n & 0x01) == 0) && (j > 1))
    {
      dash_polygon[j]=primitive_info[i-1];
      dash_polygon[j].point.x+=DrawEpsilon;
      dash_polygon[j].point.y+=DrawEpsilon;
      dash_polygon[j].coordinates=1;
      j++;
      dash_polygon[0].coordinates=(size_t) j;
      dash_polygon[j].primitive=UndefinedPrimitive;
      status&=DrawStrokePolygon(image,clone_info,dash_polygon,exception);
    }
  dash_polygon=(PrimitiveInfo *) RelinquishMagickMemory(dash_polygon);
  clone_info=DestroyDrawInfo(clone_info);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(DrawEvent,GetMagickModule(),"    end draw-dash");
  return(status != 0 ? MagickTrue : MagickFalse);
}