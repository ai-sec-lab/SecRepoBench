for (i=0; primitive_info[i].primitive != UndefinedPrimitive; i++) ;
  number_vertices=(size_t) i;
  dash_polygon=(PrimitiveInfo *) AcquireQuantumMemory((size_t)
    (2UL*number_vertices+1UL),sizeof(*dash_polygon));
  if (dash_polygon == (PrimitiveInfo *) NULL)
    return(MagickFalse);
  clone_info=CloneDrawInfo((ImageInfo *) NULL,draw_info);