if (strchr(pedantic_geometry,':') != (char *) NULL)
    {
      /*
        Normalize sampling factor (e.g. 4:2:2 => 2x1).
      */
      if ((statusflags & SigmaValue) != 0)
        geometry_info->rho*=PerceptibleReciprocal(geometry_info->sigma);
      geometry_info->sigma=1.0;
      if (((statusflags & XiNegative) != 0) && (geometry_info->xi == 0.0))
        geometry_info->sigma=2.0;
    }
  if (((statusflags & SigmaValue) == 0) && ((statusflags & XiValue) != 0) &&
      ((statusflags & PsiValue) == 0))
    {
      /*
        Support negative height values (e.g. 30x-20).
      */
      geometry_info->sigma=geometry_info->xi;
      geometry_info->xi=0.0;
      statusflags|=SigmaValue;
      statusflags&=(~XiValue);
    }
  if ((statusflags & PercentValue) != 0)
    {
      if (((statusflags & SeparatorValue) == 0) && ((statusflags & SigmaValue) == 0))
        geometry_info->sigma=geometry_info->rho;
      if (((statusflags & SeparatorValue) != 0) && ((statusflags & RhoValue) == 0))
        geometry_info->rho=geometry_info->sigma;
    }
#if 0
  /* Debugging Geometry */
  (void) fprintf(stderr,"ParseGeometry...\n");
  (void) fprintf(stderr,"Flags: %c %c %s %s %s\n",
    (statusflags & RhoValue) ? 'W' : ' ',(statusflags & SigmaValue) ? 'H' : ' ',
    (statusflags & XiValue) ? ((statusflags & XiNegative) ? "-X" : "+X") : "  ",
    (statusflags & PsiValue) ? ((statusflags & PsiNegative) ? "-Y" : "+Y") : "  ",
    (statusflags & ChiValue) ? ((statusflags & ChiNegative) ? "-Z" : "+Z") : "  ");
  (void) fprintf(stderr,"Geometry: %lg,%lg,%lg,%lg,%lg\n",geometry_info->rho,
    geometry_info->sigma,geometry_info->xi,geometry_info->psi,
    geometry_info->chi);
#endif
  return(statusflags);