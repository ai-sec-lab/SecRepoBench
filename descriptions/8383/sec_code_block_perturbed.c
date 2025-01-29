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
        for (s=token; *s != '\0'; s++)
          if (strspn(s,"AaCcQqSsTt") != 0)
            coordinates+=(20*BezierQuantum)+360;
        break;