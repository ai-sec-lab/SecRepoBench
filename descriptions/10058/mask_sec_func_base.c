MagickExport MagickBooleanType GlobExpression(const char *expression,
  const char *pattern,const MagickBooleanType case_insensitive)
{
  MagickBooleanType
    done,
    match;

  register const char
    *p;

  /*
    Return on empty pattern or '*'.
  */
  if (pattern == (char *) NULL)
    return(MagickTrue);
  if (GetUTFCode(pattern) == 0)
    return(MagickTrue);
  if (LocaleCompare(pattern,"*") == 0)
    return(MagickTrue);
  p=pattern+strlen(pattern)-1;
  if ((GetUTFCode(p) == ']') && (strchr(pattern,'[') != (char *) NULL))
    {
      ExceptionInfo
        *exception;

      ImageInfo
        *image_info;

      /*
        Determine if pattern is a scene, i.e. img0001.pcd[2].
      */
      image_info=AcquireImageInfo();
      (void) CopyMagickString(image_info->filename,pattern,MagickPathExtent);
      exception=AcquireExceptionInfo();
      (void) SetImageInfo(image_info,0,exception);
      exception=DestroyExceptionInfo(exception);
      if (LocaleCompare(image_info->filename,pattern) != 0)
        {
          image_info=DestroyImageInfo(image_info);
          return(MagickFalse);
        }
      image_info=DestroyImageInfo(image_info);
    }
  /*
    Evaluate glob expression.
  */
  done=MagickFalse;
  while ((GetUTFCode(pattern) != 0) && (done == MagickFalse))
  {
    if (GetUTFCode(expression) == 0)
      if ((GetUTFCode(pattern) != '{') && (GetUTFCode(pattern) != '*'))
        break;
    switch (GetUTFCode(pattern))
    {
      case '*':
      {
        MagickBooleanType
          status;

        status=MagickFalse;
        while (GetUTFCode(pattern) == '*')
          pattern+=GetUTFOctets(pattern);
        while ((GetUTFCode(expression) != 0) && (status == MagickFalse))
        {
          status=GlobExpression(expression,pattern,case_insensitive);
          expression+=GetUTFOctets(expression);
        }
        if (status != MagickFalse)
          {
            while (GetUTFCode(expression) != 0)
              expression+=GetUTFOctets(expression);
            while (GetUTFCode(pattern) != 0)
              pattern+=GetUTFOctets(pattern);
          }
        break;
      }
      case '[':
      {
        int
          c;

        pattern+=GetUTFOctets(pattern);
        for ( ; ; )
        {
          if ((GetUTFCode(pattern) == 0) || (GetUTFCode(pattern) == ']'))
            {
              done=MagickTrue;
              break;
            }
          if (GetUTFCode(pattern) == '\\')
            {
              pattern+=GetUTFOctets(pattern);
              if (GetUTFCode(pattern) == 0)
                {
                  done=MagickTrue;
                  break;
                }
             }
          if (GetUTFCode(pattern+GetUTFOctets(pattern)) == '-')
            {
              c=GetUTFCode(pattern);
              pattern+=GetUTFOctets(pattern);
              pattern+=GetUTFOctets(pattern);
              if (GetUTFCode(pattern) == ']')
                {
                  done=MagickTrue;
                  break;
                }
              if (GetUTFCode(pattern) == '\\')
                {
                  pattern+=GetUTFOctets(pattern);
                  if (GetUTFCode(pattern) == 0)
                    {
                      done=MagickTrue;
                      break;
                    }
                }
              if ((GetUTFCode(expression) < c) ||
                  (GetUTFCode(expression) > GetUTFCode(pattern)))
                {
                  pattern+=GetUTFOctets(pattern);
                  continue;
                }
            }
          else
            if (GetUTFCode(pattern) != GetUTFCode(expression))
              {
                pattern+=GetUTFOctets(pattern);
                continue;
              }
          pattern+=GetUTFOctets(pattern);
          while ((GetUTFCode(pattern) != ']') && (GetUTFCode(pattern) != 0))
          {
            if ((GetUTFCode(pattern) == '\\') &&
                (GetUTFCode(pattern+GetUTFOctets(pattern)) > 0))
              pattern+=GetUTFOctets(pattern);
            pattern+=GetUTFOctets(pattern);
          }
          if (GetUTFCode(pattern) != 0)
            {
              pattern+=GetUTFOctets(pattern);
              expression+=GetUTFOctets(expression);
            }
          break;
        }
        break;
      }
      case '?':
      {
        pattern+=GetUTFOctets(pattern);
        expression+=GetUTFOctets(expression);
        break;
      }
      case '{':
      {
        char
          *target;

        register char
          *p;

        target=AcquireString(pattern);
        p=target;
        while ((GetUTFCode(pattern) != '}') && (GetUTFCode(pattern) != 0))
        {
          *p++=(*pattern++);
          if ((GetUTFCode(pattern) == ',') || (GetUTFCode(pattern) == '}'))
            {
              *p='\0';
              // <MASK>
              pattern+=GetUTFOctets(pattern);
            }
        }
        while ((GetUTFCode(pattern) != '}') && (GetUTFCode(pattern) != 0))
          pattern+=GetUTFOctets(pattern);
        if (GetUTFCode(pattern) != 0)
          pattern+=GetUTFOctets(pattern);
        target=DestroyString(target);
        break;
      }
      case '\\':
      {
        pattern+=GetUTFOctets(pattern);
        if (GetUTFCode(pattern) == 0)
          break;
      }
      default:
      {
        if (case_insensitive != MagickFalse)
          {
            if (tolower((int) GetUTFCode(expression)) !=
                tolower((int) GetUTFCode(pattern)))
              {
                done=MagickTrue;
                break;
              }
          }
        else
          if (GetUTFCode(expression) != GetUTFCode(pattern))
            {
              done=MagickTrue;
              break;
            }
        expression+=GetUTFOctets(expression);
        pattern+=GetUTFOctets(pattern);
      }
    }
  }
  while (GetUTFCode(pattern) == '*')
    pattern+=GetUTFOctets(pattern);
  match=(GetUTFCode(expression) == 0) && (GetUTFCode(pattern) == 0) ?
    MagickTrue : MagickFalse;
  return(match);
}