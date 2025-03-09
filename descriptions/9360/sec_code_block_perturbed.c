end=q;
            (void) CopyMagickString(name,token,MagickPathExtent);
            n=0;
            for (p=q; *q != '\0'; )
            {
              GetNextToken(p,&p,length,token);
              if (*token == '\0')
                break;
              if (*token == '#')
                {
                  /*
                    Skip comment.
                  */
                  while ((*p != '\n') && (*p != '\0'))
                    p++;
                  continue;
                }
              if (LocaleCompare(token,"pop") == 0)
                {
                  end=p-strlen(token)-1;
                  n--;
                }
              if (LocaleCompare(token,"push") == 0)
                n++;
              if ((n < 0) && (end > start))
                {
                  char
                    *macro;

                  /*
                    Extract macro.
                  */
                  GetNextToken(p,&p,length,token);
                  macro=AcquireString(start);
                  macro[end-start]='\0';
                  (void) AddValueToSplayTree(macros,ConstantString(name),
                    ConstantString(macro));
                  macro=DestroyString(macro);
                  break;
                }
            }