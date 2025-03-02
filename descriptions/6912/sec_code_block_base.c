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