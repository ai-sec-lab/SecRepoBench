
          GeometryInfo
            geometry_info;

          flags=ParseGeometry(value,&geometry_info);
          image->resolution.x=geometry_info.rho;
          image->resolution.y=geometry_info.sigma;
          if ((flags & SigmaValue) == 0)
            image->resolution.y=image->resolution.x;
          return(MagickTrue);
        