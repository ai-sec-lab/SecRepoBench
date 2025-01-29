#define CheckPathExtent(pad) \
  if ((ssize_t) (p+(pad)) >= (ssize_t) max_strokes) \
    { \
      if (~max_strokes < (pad)) \
        path_p=(PointInfo *) RelinquishMagickMemory(path_p); \
      else \
        { \
          max_strokes+=(pad); \
          path_p=(PointInfo *) ResizeQuantumMemory(path_p,max_strokes+ \
            MaxStrokePad,sizeof(*path_p)); \
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
    } \
  if ((ssize_t) (q+(pad)) >= (ssize_t) max_strokes) \
    { \
      if (~max_strokes < (pad)) \
        path_q=(PointInfo *) RelinquishMagickMemory(path_q); \
      else \
        { \
          max_strokes+=(pad); \
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