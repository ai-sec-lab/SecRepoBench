static char *GetNodeByID(const char *primitive,const char *id)
{
  char
    *token;

  const char
    *q,
    *start;

  register const char
    *p;

  size_t
    maxextent,
    length;

  ssize_t
    n;

  /*
    Find and return group by ID.
  */
  token=AcquireString(primitive);
  maxextent=strlen(token)+MagickPathExtent;
  length=0;
  n=0;
  start=(const char *) NULL;
  p=(const char *) NULL;
  for (q=primitive; (*q != '\0') && (length == 0); )
  {
    p=q;
    GetNextToken(q,&q,maxextent,token);
    if (*token == '\0')
      break;
    if (*token == '#')
      {
        /*
          Comment.
        */
        while ((*q != '\n') && (*q != '\0'))
          q++;
        continue;
      }
    if (LocaleCompare("pop",token) == 0)
      {
        GetNextToken(q,&q,maxextent,token);
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
      }
    if (LocaleCompare("push",token) == 0)
      {
        GetNextToken(q,&q,maxextent,token);
        if (LocaleCompare("graphic-context",token) == 0)
          {
            n++;
            if (*q == '"')
              {
                GetNextToken(q,&q,maxextent,token);
                if (LocaleCompare(id,token) == 0)
                  {
                    /*
                      Start of group by ID.
                    */
                    n=0;
                    start=q;
                  }
              }
          }
      }
  }
  if (start == (const char *) NULL)
    return((char *) NULL);
  (void) CopyMagickString(token,start,length);
  return(token);
}