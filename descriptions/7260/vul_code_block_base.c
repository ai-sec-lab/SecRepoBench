case CirclePrimitive:
      case ArcPrimitive:
      {
        PointInfo
          degrees;

        degrees.x=0.0;
        degrees.y=360.0;
        coordinates=GetEllipseCoordinates(primitive_info[j].point,
          primitive_info[j+1].point,degrees);
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