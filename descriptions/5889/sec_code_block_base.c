case CirclePrimitive:
      case ArcPrimitive:
      case EllipsePrimitive:
      {
        double
          alpha,
          beta,
          radius;

        alpha=bounds.x2-bounds.x1;
        beta=bounds.y2-bounds.y1;
        radius=hypot(alpha,beta);
        points_extent=ceil(MagickPI*MagickPI*radius)+6*BezierQuantum+360;
        break;
      }