#define CheckPathExtent(pad) \
  if ((ssize_t) (q+(pad)) >= (ssize_t) stroke_limit) \
    { \
      if (~stroke_limit < (pad)) \
        { \
          path_p=(PointInfo *) RelinquishMagickMemory(path_p); \
          path_q=(PointInfo *) RelinquishMagickMemory(path_q); \
        } \
      else \
        { \
          stroke_limit+=(pad); \
          path_p=(PointInfo *) ResizeQuantumMemory(path_p,stroke_limit+ \
            MaxStrokePad,sizeof(*path_p)); \
          path_q=(PointInfo *) ResizeQuantumMemory(path_q,stroke_limit+ \
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