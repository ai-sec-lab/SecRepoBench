/*
          Move to.
        */
        if (mvg_info->offset != subpath_offset)
          {
            primitive_info=(*mvg_info->primitive_info)+subpath_offset;
            primitive_info->coordinates=(size_t) (q-primitive_info);
            number_coordinates+=primitive_info->coordinates;
            primitive_info=q;
            subpath_offset=mvg_info->offset;
          }
        i=0;
        do
        {
          GetNextToken(p,&p,MagickPathExtent,token);
          if (*token == ',')
            GetNextToken(p,&p,MagickPathExtent,token);
          x=StringToDouble(token,&next_token);
          if (token == next_token)
            ThrowPointExpectedException(token,exception);
          GetNextToken(p,&p,MagickPathExtent,token);
          if (*token == ',')
            GetNextToken(p,&p,MagickPathExtent,token);
          y=StringToDouble(token,&next_token);
          if (token == next_token)
            ThrowPointExpectedException(token,exception);
          point.x=(double) (attribute == (int) 'M' ? x : point.x+x);
          point.y=(double) (attribute == (int) 'M' ? y : point.y+y);
          if (i == 0)
            start=point;
          i++;
          if (((size_t) (mvg_info->offset+4096) > *mvg_info->extent) &&
              (CheckPrimitiveExtent(mvg_info,4096) != MagickFalse))
            q=(*mvg_info->primitive_info)+mvg_info->offset;
          TracePoint(q,point);
          mvg_info->offset+=q->coordinates;
          q+=q->coordinates;
          while (isspace((int) ((unsigned char) *p)) != 0)
            p++;
          if (*p == ',')
            p++;
        } while (IsPoint(p) != MagickFalse);
        break;