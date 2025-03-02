level++;
              q=subexpression;
              for (p++; *p != '\0'; )
              {
                if (*p == '[')
                  level++;
                else
                  if (*p == ']')
                    {
                      level--;
                      if (level == 0)
                        break;
                    }
                *q++=(*p++);
              }
              *q='\0';
              alpha=FxEvaluateSubexpression(fx_info,channel,x,y,subexpression,
                &depth,&beta,exception);
              i=(ssize_t) alpha;
              p++;