static SplayTreeInfo *GetMVGMacros(const char *primitive)
{
  char
    *token;

  const char
    *q;

  size_t
    length;

  SplayTreeInfo
    *macros;

  /*
    Scan graphic primitives for definitions and classes.
  */
  if (primitive == (const char *) NULL)
    return((SplayTreeInfo *) NULL);
  macros=NewSplayTree(CompareSplayTreeString,RelinquishMagickMemory,
    RelinquishMagickMemory);
  token=AcquireString(primitive);
  length=strlen(token)+MagickPathExtent;
  for (q=primitive; *q != '\0'; )
  {
    GetNextToken(q,&q,length,token);
    if (*token == '\0')
      break;
    if (*token == '#')
      {
        /*
          Skip comment.
        */
        while ((*q != '\n') && (*q != '\0'))
          q++;
        continue;
      }
    if (LocaleCompare("push",token) == 0)
      {
        register const char
          *end,
          *start;

        GetNextToken(q,&q,length,token);
        if (*q == '"')
          {
            char
              name[MagickPathExtent];

            const char
              *p;

            ssize_t
             n;

            /*
              Named macro (e.g. push graphic-context "wheel").
            */
            GetNextToken(q,&q,length,token);
            start=q;
            // <MASK>
          }
      }
  }
  token=DestroyString(token);
  return(macros);
}