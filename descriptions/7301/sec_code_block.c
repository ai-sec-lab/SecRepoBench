
      case RectanglePrimitive:
      {
        coordinates*=5;
        break;
      }
      case RoundRectanglePrimitive:
      {
        coordinates=GetRoundRectangleCoordinates(primitive_info[j].point,
          primitive_info[j+1].point,primitive_info[j+2].point);
        break;
      }
      case BezierPrimitive:
      {
        if (primitive_info[j].coordinates > 107)
          {
            (void) ThrowMagickException(exception,GetMagickModule(),DrawError,
              "TooManyBezierCoordinates","`%s'",token);
            status=MagickFalse;
            break;
          }
        coordinates=BezierQuantum*primitive_info[j].coordinates;
        break;
      }
      case PathPrimitive:
      {
        char
          *s,
          *t;

        GetNextToken(q,&q,extent,token);
        coordinates=1;
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
          coordinates++;
        }
        coordinates*=(6*BezierQuantum)+360;
        break;
      }
      case CirclePrimitive:
      {
        PointInfo
          degrees;

        degrees.x=0.0;
        degrees.y=360.0;
        coordinates=GetEllipseCoordinates(primitive_info[j].point,
          primitive_info[j+1].point,degrees);
        break;
      }
      case ArcPrimitive:
      {
        PointInfo
          center,
          radii;

        center.x=0.5*(primitive_info[j+1].point.x+primitive_info[j].point.x);
        center.y=0.5*(primitive_info[j+1].point.y+primitive_info[j].point.y);
        radii.x=fabs(center.x-primitive_info[j].point.x);
        radii.y=fabs(center.y-primitive_info[j].point.y);
        coordinates=GetEllipseCoordinates(center,radii,
          primitive_info[j+2].point);
        break;
      }
      case EllipsePrimitive:
      {
        coordinates=GetEllipseCoordinates(primitive_info[j].point,
          primitive_info[j+1].point,primitive_info[j+2].point);
        break;
      }
      default:
        break;
    