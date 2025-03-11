case CirclePrimitive:
      case ArcPrimitive:
      case EllipsePrimitive:
      {
        double
          alphaprimitive,
          beta,
          radius;

        alphaprimitive=bounds.x2-bounds.x1;
        beta=bounds.y2-bounds.y1;
        radius=hypot(alphaprimitive,beta);
        points_extent=ceil(MagickPI*MagickPI*radius)+6*BezierQuantum+360;
        break;
      }