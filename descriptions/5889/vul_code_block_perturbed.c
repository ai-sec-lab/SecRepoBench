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
        radius=hypot((double) alphaprimitive,(double) beta);
        points_extent=2*ceil((double) MagickPI*radius)+6*BezierQuantum+360;
        break;
      }