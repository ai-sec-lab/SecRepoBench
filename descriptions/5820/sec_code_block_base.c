if (strchr(pedantic_geometry,':') != (char *) NULL)
    {
      /*
        Normalize sampling factor (e.g. 4:2:2 => 2x1).
      */
      if ((flags & SigmaValue) != 0)
        geometry_info->rho*=PerceptibleReciprocal(geometry_info->sigma);
      geometry_info->sigma=1.0;
      if (((flags & XiValue) != 0) && (geometry_info->xi == 0.0))
        geometry_info->sigma=2.0;
    }
  if (((flags & SigmaValue) == 0) && ((flags & XiValue) != 0) &&
      ((flags & PsiValue) == 0))
    {
      /*
        Support negative height values (e.g. 30x-20).
      */
      geometry_info->sigma=geometry_info->xi;
      geometry_info->xi=0.0;
      flags|=SigmaValue;
      flags&=(~XiValue);
    }
  if ((flags & PercentValue) != 0)
    {
      if (((flags & SeparatorValue) == 0) && ((flags & SigmaValue) == 0))
        geometry_info->sigma=geometry_info->rho;
      if (((flags & SeparatorValue) != 0) && ((flags & RhoValue) == 0))
        geometry_info->rho=geometry_info->sigma;
    }
#if 0
  /* Debugging Geometry */
  (void) fprintf(stderr,"ParseGeometry...\n");
  (void) fprintf(stderr,"Flags: %c %c %s %s %s\n",
    (flags & RhoValue) ? 'W' : ' ',(flags & SigmaValue) ? 'H' : ' ',
    (flags & XiValue) ? ((flags & XiNegative) ? "-X" : "+X") : "  ",
    (flags & PsiValue) ? ((flags & PsiNegative) ? "-Y" : "+Y") : "  ",
    (flags & ChiValue) ? ((flags & ChiNegative) ? "-Z" : "+Z") : "  ");
  (void) fprintf(stderr,"Geometry: %lg,%lg,%lg,%lg,%lg\n",geometry_info->rho,
    geometry_info->sigma,geometry_info->xi,geometry_info->psi,
    geometry_info->chi);
#endif
  return(flags);