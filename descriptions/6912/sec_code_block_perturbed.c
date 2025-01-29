case PathPrimitive:
      {
        char
          *s,
          *t;

        GetNextToken(q,&q,extent,token);
        points_extent=1;
        t=token;
        for (s=token; *s != '\0'; s=t)
        {
          double
            value;

          value=StringToDouble(s,&t);
          (void) value;
          if (s == t)
            {
              t++;
              continue;
            }
          points_extent++;
        }
        points_extent*=(6*BezierQuantum)+360.0;
        break;
      }
      case CirclePrimitive:
      case ArcPrimitive:
      {
        double
          alpha,
          beta,
          coordinates,
          radius;

        alpha=bounds.x2-bounds.x1;
        beta=bounds.y2-bounds.y1;
        radius=hypot(alpha,beta);
        coordinates=ceil(MagickPI*MagickPI*radius)+6*BezierQuantum+360;
        if (coordinates > 21438)
          {
            (void) ThrowMagickException(exception,GetMagickModule(),DrawError,
              "TooManyBezierCoordinates","`%s'",token);
            status=MagickFalse;
            break;
          }
        points_extent=2*coordinates;
        break;
      }
      case EllipsePrimitive:
      {
        points_extent=(double) EllipsePoints(primitive_info+j,
          primitive_info[j].point,primitive_info[j+1].point,
          primitive_info[j+2].point);
        break;
      }
      default:
        break;