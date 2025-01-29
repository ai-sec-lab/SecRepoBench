match=GlobExpression(expression,target,case_insensitive);
              if (match != MagickFalse)
                {
                  expression+=MagickMin(strlen(expression),strlen(target));
                  break;
                }
              p=target;