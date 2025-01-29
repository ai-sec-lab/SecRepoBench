match=GlobExpression(expression,target,case_insensitive);
              if (match != MagickFalse)
                {
                  expression+=strlen(target);
                  break;
                }
              p=target;