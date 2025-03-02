static PathInfo *ConvertPrimitiveToPath(const PrimitiveInfo *primitive_info)
{
  PathInfo
    *path_info;

  PathInfoCode
    code;

  PointInfo
    p,
    q;

  register ssize_t
    i,
    n;

  ssize_t
    coordinates,
    start;

  /*
    Converts a PrimitiveInfo structure into a vector path structure.
  */
  // <MASK>
  for (i=0; primitive_info[i].primitive != UndefinedPrimitive; i++)
  {
    code=LineToCode;
    if (coordinates <= 0)
      {
        coordinates=(ssize_t) primitive_info[i].coordinates;
        p=primitive_info[i].point;
        start=n;
        code=MoveToCode;
      }
    coordinates--;
    /*
      Eliminate duplicate points.
    */
    if ((code == MoveToCode) || (fabs(q.x-primitive_info[i].point.x) >= DrawEpsilon) ||
        (fabs(q.y-primitive_info[i].point.y) >= DrawEpsilon))
      {
        path_info[n].code=code;
        path_info[n].point=primitive_info[i].point;
        q=primitive_info[i].point;
        n++;
      }
    if (coordinates > 0)
      continue;
    if ((fabs(p.x-primitive_info[i].point.x) < DrawEpsilon) &&
        (fabs(p.y-primitive_info[i].point.y) < DrawEpsilon))
      continue;
    /*
      Mark the p point as open if it does not match the q.
    */
    path_info[start].code=OpenCode;
    path_info[n].code=GhostlineCode;
    path_info[n].point=primitive_info[i].point;
    n++;
    path_info[n].code=LineToCode;
    path_info[n].point=p;
    n++;
  }
  path_info[n].code=EndCode;
  path_info[n].point.x=0.0;
  path_info[n].point.y=0.0;
  if (IsEventLogging() != MagickFalse)
    LogPathInfo(path_info);
  return(path_info);
}