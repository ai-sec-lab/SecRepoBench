#define MaxStrokePad  (6*BezierQuantum+360)
#define CheckPathExtent(pad) \
  if ((ssize_t) (q+(pad)) >= (ssize_t) max_strokes) \
    { \
      if (~max_strokes < (pad)) \
        { \
          path_p=(PointInfo *) RelinquishMagickMemory(path_p); \
          path_q=(PointInfo *) RelinquishMagickMemory(path_q); \
        } \
      else \
        { \
          max_strokes+=(pad); \
          path_p=(PointInfo *) ResizeQuantumMemory(path_p,max_strokes+ \
            MaxStrokePad,sizeof(*path_p)); \
          path_q=(PointInfo *) ResizeQuantumMemory(path_q,max_strokes+ \
            MaxStrokePad,sizeof(*path_q)); \
        } \
      if ((path_p == (PointInfo *) NULL) || (path_q == (PointInfo *) NULL)) \
        { \
          if (path_p != (PointInfo *) NULL) \
            path_p=(PointInfo *) RelinquishMagickMemory(path_p); \
          if (path_q != (PointInfo *) NULL) \
            path_q=(PointInfo *) RelinquishMagickMemory(path_q); \
          polygon_primitive=(PrimitiveInfo *) \
            RelinquishMagickMemory(polygon_primitive); \
          return((PrimitiveInfo *) NULL); \
        } \
    }

  typedef struct _LineSegment
  {
    double
      p,
      q;
  } LineSegment;

  double
    delta_theta,
    dot_product,
    mid,
    miterlimit;

  LineSegment
    dx = {0,0},
    dy = {0,0},
    inverse_slope = {0,0},
    slope = {0,0},
    theta = {0,0};

  MagickBooleanType
    closed_path;

  PointInfo
    box_p[5],
    box_q[5],
    center,
    offset,
    *path_p,
    *path_q;

  PrimitiveInfo
    *polygon_primitive,
    *stroke_polygon;

  register ssize_t
    i;

  size_t
    arc_segments,
    max_strokes,
    number_vertices;

  ssize_t
    j,
    n,
    p,
    q;

  /*
    Allocate paths.
  */
  number_vertices=primitivedata->coordinates;
  max_strokes=2*number_vertices;
  polygon_primitive=(PrimitiveInfo *) AcquireQuantumMemory((size_t)
    number_vertices+2UL,sizeof(*polygon_primitive));
  if (polygon_primitive == (PrimitiveInfo *) NULL)
    return((PrimitiveInfo *) NULL);
  (void) memcpy(polygon_primitive,primitivedata,(size_t) number_vertices*
    sizeof(*polygon_primitive));
  closed_path=primitivedata[0].closed_subpath;
  if (((draw_info->linejoin == RoundJoin) ||
       (draw_info->linejoin == MiterJoin)) && (closed_path != MagickFalse))
    {
      polygon_primitive[number_vertices]=primitivedata[1];
      number_vertices++;
    }
  polygon_primitive[number_vertices].primitive=UndefinedPrimitive;
  /*
    Compute the slope for the first line segment, p.
  */
  dx.p=0.0;
  dy.p=0.0;
  for (n=1; n < (ssize_t) number_vertices; n++)
  {
    dx.p=polygon_primitive[n].point.x-polygon_primitive[0].point.x;
    dy.p=polygon_primitive[n].point.y-polygon_primitive[0].point.y;
    if ((fabs(dx.p) >= MagickEpsilon) || (fabs(dy.p) >= MagickEpsilon))
      break;
  }
  if (n == (ssize_t) number_vertices)
    {
      if ((draw_info->linecap != RoundCap) || (closed_path != MagickFalse))
        {
          /*
            Zero length subpath.
          */
          stroke_polygon=(PrimitiveInfo *) AcquireCriticalMemory(
            sizeof(*stroke_polygon));
          stroke_polygon[0]=polygon_primitive[0];
          stroke_polygon[0].coordinates=0;
          polygon_primitive=(PrimitiveInfo *) RelinquishMagickMemory(
            polygon_primitive);
          return(stroke_polygon);
        }
      n=(ssize_t) number_vertices-1L;
    }
  path_p=(PointInfo *) AcquireQuantumMemory((size_t) max_strokes+MaxStrokePad,
    sizeof(*path_p));
  if (path_p == (PointInfo *) NULL)
    {
      polygon_primitive=(PrimitiveInfo *) RelinquishMagickMemory(
        polygon_primitive);
      return((PrimitiveInfo *) NULL);
    }
  path_q=(PointInfo *) AcquireQuantumMemory((size_t) max_strokes+MaxStrokePad,
    sizeof(*path_q));