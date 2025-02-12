if ((flags & RhoValue) != 0)
            image->resolution.x=geometry_info.rho;
          image->resolution.y=image->resolution.x;
          if ((flags & SigmaValue) != 0)
            image->resolution.y=geometry_info.sigma;