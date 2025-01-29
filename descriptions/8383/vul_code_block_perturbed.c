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
          if (strspn(t,"AaCcQqSsTt") != 0)
            coordinates+=(6*BezierQuantum)+360;
          coordinates++;
        }
        break;