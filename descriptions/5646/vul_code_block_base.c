token[i]='\0';
  if ((LocaleNCompare(token,"url(",4) == 0) && (strlen(token) > 4))
    {
      ssize_t
        offset;

      offset=4;
      if (token[offset] == '#')
        offset++;
      i=(ssize_t) strlen(token);
      (void) CopyMagickString(token,token+offset,MagickPathExtent);
      token[i-offset-1]='\0';
    }
  while (isspace((int) ((unsigned char) *p)) != 0)
    p++;
  if (end != (const char **) NULL)
    *end=(const char *) p;