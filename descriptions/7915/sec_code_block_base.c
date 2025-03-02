switch (primitive_info->primitive)
  {
    case AlphaPrimitive:
    case ColorPrimitive:
    case ImagePrimitive:
    case PointPrimitive:
    case TextPrimitive:
      return((PathInfo *) NULL);
    default:
      break;
  }
  for (i=0; primitive_info[i].primitive != UndefinedPrimitive; i++) ;
  path_info=(PathInfo *) AcquireQuantumMemory((size_t) (2UL*i+4UL),
    sizeof(*path_info));
  if (path_info == (PathInfo *) NULL)
    return((PathInfo *) NULL);
  coordinates=0;
  n=0;
  p.x=(-1.0);
  p.y=(-1.0);
  q.x=(-1.0);
  q.y=(-1.0);
  start=0;