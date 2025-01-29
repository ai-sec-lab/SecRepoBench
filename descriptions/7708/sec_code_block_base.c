if (LocaleCompare("graphic-context",token) == 0)
          if (n == 0)
            {
              /*
                End of group by ID.
              */
              if (start != (const char *) NULL)
                length=(size_t) (p-start+1);
              break;
            }
        n--;