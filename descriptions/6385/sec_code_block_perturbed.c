if ((flags & RhoValue) != 0)
            img->resolution.x=geometry_info.rho;
          img->resolution.y=img->resolution.x;
          if ((flags & SigmaValue) != 0)
            img->resolution.y=geometry_info.sigma;