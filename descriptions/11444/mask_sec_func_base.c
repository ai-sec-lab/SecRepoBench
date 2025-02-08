MagickExport void GetNextToken(const char *start,const char **end,
  const size_t extent,char *token)
{
  double
    value;

  register char
    *q;

  register const char
    *p;

  register ssize_t
    i;

  assert(start != (const char *) NULL);
  assert(token != (char *) NULL);
  i=0;
  p=start;
  while ((isspace((int) ((unsigned char) *p)) != 0) && (*p != '\0'))
    p++;
  switch (*p)
  {
    case '\0':
      break;
    case '"':
    case '\'':
    case '`':
    case '{':
    {
      register char
        escape;

      switch (*p)
      {
        case '"': escape='"'; break;
        case '\'': escape='\''; break;
        case '`': escape='\''; break;
        case '{': escape='}'; break;
        default: escape=(*p); break;
      }
      for (p++; *p != '\0'; p++)
      {
        if ((*p == '\\') && ((*(p+1) == escape) || (*(p+1) == '\\')))
          p++;
        else
          if (*p == escape)
            {
              p++;
              break;
            }
        if (i < (ssize_t) (extent-1))
          token[i++]=(*p);
        if ((size_t) (p-start) >= (extent-1))
          break;
      }
      break;
    }
    case '/':
    {
      if (i < (ssize_t) (extent-1))
        token[i++]=(*p);
      p++;
      if ((*p == '>') || (*p == '/'))
        {
          if (i < (ssize_t) (extent-1))
            token[i++]=(*p);
          p++;
        }
      break;
    }
    default:
    {
      char
        *q;

      value=StringToDouble(p,&q);
      (void) value;
      if ((p != q) && (*p != ','))
        {
          for ( ; (p < q) && (*p != ','); p++)
          {
            if (i < (ssize_t) (extent-1))
              token[i++]=(*p);
            if ((size_t) (p-start) >= (extent-1))
              break;
          }
          if (*p == '%')
            {
              if (i < (ssize_t) (extent-1))
                token[i++]=(*p);
              p++;
            }
          break;
        }
      if ((*p != '\0') && (isalpha((int) ((unsigned char) *p)) == 0) &&
          (*p != *DirectorySeparator) && (*p != '#') && (*p != '<'))
        {
          if (i < (ssize_t) (extent-1))
            token[i++]=(*p);
          p++;
          break;
        }
      for ( ; *p != '\0'; p++)
      {
        if (((isspace((int) ((unsigned char) *p)) != 0) || (*p == '=') ||
            (*p == ',') || (*p == ':') || (*p == ';')) && (*(p-1) != '\\'))
          break;
        if ((i > 0) && (*p == '<'))
          break;
        if (i < (ssize_t) (extent-1))
          token[i++]=(*p);
        if (*p == '>')
          break;
        // <MASK>
        if ((size_t) (p-start) >= (extent-1))
          break;
      }
      break;
    }
  }
  token[i]='\0';
  q=strrchr(token,')');
  if ((LocaleNCompare(token,"url(#",5) == 0) && (q != (char *) NULL))
    {
      *q='\0';
      (void) memmove(token,token+5,(size_t) (q-token-4));
    }
  while (isspace((int) ((unsigned char) *p)) != 0)
    p++;
  if (end != (const char **) NULL)
    *end=(const char *) p;
}