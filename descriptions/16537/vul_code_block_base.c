  quantum=MagickMin(quantum/number_coordinates,BezierQuantum);
  primitive_info=(*mvg_info->primitive_info)+mvg_info->offset;
  coefficients=(double *) AcquireQuantumMemory(number_coordinates,
    sizeof(*coefficients));
  points=(PointInfo *) AcquireQuantumMemory(quantum,number_coordinates*
    sizeof(*points));
  if ((coefficients == (double *) NULL) || (points == (PointInfo *) NULL))
    {
      if (points != (PointInfo *) NULL)
        points=(PointInfo *) RelinquishMagickMemory(points);
      if (coefficients != (double *) NULL)
        coefficients=(double *) RelinquishMagickMemory(coefficients);
      (void) ThrowMagickException(mvg_info->exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'","");
      return(MagickFalse);
    }
  control_points=quantum*number_coordinates;
  if (CheckPrimitiveExtent(mvg_info,control_points+1) == MagickFalse)
    {
      points=(PointInfo *) RelinquishMagickMemory(points);
      coefficients=(double *) RelinquishMagickMemory(coefficients);
      return(MagickFalse);
    }