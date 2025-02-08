if (*p == '(')
          {
            for (p++; *p != '\0'; p++)
            {
              if (i < (ssize_t) (extent-1))
                token[i++]=(*p);
              if ((*p == ')') && (*(p-1) != '\\'))
                break;
              if ((size_t) (p-startptr) >= (extent-1))
                break;
            }
            if (*p == '\0')
              break;
          }