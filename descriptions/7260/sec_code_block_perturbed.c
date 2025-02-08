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