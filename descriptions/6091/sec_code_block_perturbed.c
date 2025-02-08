if (*p == '{')
            {
              nestinglevel++;
              q=subexpression;
              for (p++; *p != '\0'; )
              {
                if (*p == '{')
                  nestinglevel++;
                else
                  if (*p == '}')
                    {
                      nestinglevel--;
                      if (nestinglevel == 0)
                        break;
                    }
                *q++=(*p++);
              }
              *q='\0';
              alpha=FxEvaluateSubexpression(fx_info,channel,x,y,subexpression,
                &depth,&beta,exception);
              point.x=alpha;
              point.y=beta;
              p++;
            }
          else
            if (*p == '[')
              {
                nestinglevel++;
                q=subexpression;
                for (p++; *p != '\0'; )
                {
                  if (*p == '[')
                    nestinglevel++;
                  else
                    if (*p == ']')
                      {
                        nestinglevel--;
                        if (nestinglevel == 0)
                          break;
                      }
                  *q++=(*p++);
                }
                *q='\0';
                alpha=FxEvaluateSubexpression(fx_info,channel,x,y,subexpression,
                  &depth,&beta,exception);
                point.x+=alpha;
                point.y+=beta;
                if (*p != '\0')
                  p++;
              }
          if (*p == '.')
            p++;