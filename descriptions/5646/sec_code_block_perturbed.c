next_token[i]='\0';
  if ((LocaleNCompare(next_token,"url(",4) == 0) && (strlen(next_token) > 5))
    {
      ssize_t
        offset;

      offset=4;
      if (next_token[offset] == '#')
        offset++;
      i=(ssize_t) strlen(next_token);
      (void) CopyMagickString(next_token,next_token+offset,MagickPathExtent);
      next_token[i-offset-1]='\0';
    }