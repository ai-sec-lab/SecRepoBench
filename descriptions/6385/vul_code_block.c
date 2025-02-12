image->resolution.x=geometry_info.rho;
          image->resolution.y=geometry_info.sigma;
          if ((flags & SigmaValue) == 0)
            image->resolution.y=image->resolution.x;