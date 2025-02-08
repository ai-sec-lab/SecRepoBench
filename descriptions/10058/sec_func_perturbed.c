MagickExport MagickBooleanType GlobExpression(const char *expression,
  const char *patternstr,const MagickBooleanType case_insensitive)
{
  MagickBooleanType
    done,
    match;

  register const char
    *p;

  /*
    Return on empty pattern or '*'.
  */
  if (patternstr == (char *) NULL)
    return(MagickTrue);
  if (GetUTFCode(patternstr) == 0)
    return(MagickTrue);
  if (LocaleCompare(patternstr,"*") == 0)
    return(MagickTrue);
  p=patternstr+strlen(patternstr)-1;
  if ((GetUTFCode(p) == ']') && (strchr(patternstr,'[') != (char *) NULL))
    {
      ExceptionInfo
        *exception;

      ImageInfo
        *image_info;

      /*
        Determine if pattern is a scene, i.e. img0001.pcd[2].
      */
      image_info=AcquireImageInfo();
      (void) CopyMagickString(image_info->filename,patternstr,MagickPathExtent);
      exception=AcquireExceptionInfo();
      (void) SetImageInfo(image_info,0,exception);
      exception=DestroyExceptionInfo(exception);
      if (LocaleCompare(image_info->filename,patternstr) != 0)
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
  while ((GetUTFCode(patternstr) != 0) && (done == MagickFalse))
  {
    if (GetUTFCode(expression) == 0)
      if ((GetUTFCode(patternstr) != '{') && (GetUTFCode(patternstr) != '*'))
        break;
    switch (GetUTFCode(patternstr))
    {
      case '*':
      {
        MagickBooleanType
          status;

        status=MagickFalse;
        while (GetUTFCode(patternstr) == '*')
          patternstr+=GetUTFOctets(patternstr);
        while ((GetUTFCode(expression) != 0) && (status == MagickFalse))
        {
          status=GlobExpression(expression,patternstr,case_insensitive);
          expression+=GetUTFOctets(expression);
        }
        if (status != MagickFalse)
          {
            while (GetUTFCode(expression) != 0)
              expression+=GetUTFOctets(expression);
            while (GetUTFCode(patternstr) != 0)
              patternstr+=GetUTFOctets(patternstr);
          }
        break;
      }
      case '[':
      {
        int
          c;

        patternstr+=GetUTFOctets(patternstr);
        for ( ; ; )
        {
          if ((GetUTFCode(patternstr) == 0) || (GetUTFCode(patternstr) == ']'))
            {
              done=MagickTrue;
              break;
            }
          if (GetUTFCode(patternstr) == '\\')
            {
              patternstr+=GetUTFOctets(patternstr);
              if (GetUTFCode(patternstr) == 0)
                {
                  done=MagickTrue;
                  break;
                }
             }
          if (GetUTFCode(patternstr+GetUTFOctets(patternstr)) == '-')
            {
              c=GetUTFCode(patternstr);
              patternstr+=GetUTFOctets(patternstr);
              patternstr+=GetUTFOctets(patternstr);
              if (GetUTFCode(patternstr) == ']')
                {
                  done=MagickTrue;
                  break;
                }
              if (GetUTFCode(patternstr) == '\\')
                {
                  patternstr+=GetUTFOctets(patternstr);
                  if (GetUTFCode(patternstr) == 0)
                    {
                      done=MagickTrue;
                      break;
                    }
                }
              if ((GetUTFCode(expression) < c) ||
                  (GetUTFCode(expression) > GetUTFCode(patternstr)))
                {
                  patternstr+=GetUTFOctets(patternstr);
                  continue;
                }
            }
          else
            if (GetUTFCode(patternstr) != GetUTFCode(expression))
              {
                patternstr+=GetUTFOctets(patternstr);
                continue;
              }
          patternstr+=GetUTFOctets(patternstr);
          while ((GetUTFCode(patternstr) != ']') && (GetUTFCode(patternstr) != 0))
          {
            if ((GetUTFCode(patternstr) == '\\') &&
                (GetUTFCode(patternstr+GetUTFOctets(patternstr)) > 0))
              patternstr+=GetUTFOctets(patternstr);
            patternstr+=GetUTFOctets(patternstr);
          }
          if (GetUTFCode(patternstr) != 0)
            {
              patternstr+=GetUTFOctets(patternstr);
              expression+=GetUTFOctets(expression);
            }
          break;
        }
        break;
      }
      case '?':
      {
        patternstr+=GetUTFOctets(patternstr);
        expression+=GetUTFOctets(expression);
        break;
      }
      case '{':
      {
        char
          *target;

        register char
          *p;

        target=AcquireString(patternstr);
        p=target;
        while ((GetUTFCode(patternstr) != '}') && (GetUTFCode(patternstr) != 0))
        {
          *p++=(*patternstr++);
          if ((GetUTFCode(patternstr) == ',') || (GetUTFCode(patternstr) == '}'))
            {
              *p='\0';
              match=GlobExpression(expression,target,case_insensitive);
              if (match != MagickFalse)
                {
                  expression+=MagickMin(strlen(expression),strlen(target));
                  break;
                }
              p=target;
              patternstr+=GetUTFOctets(patternstr);
            }
        }
        while ((GetUTFCode(patternstr) != '}') && (GetUTFCode(patternstr) != 0))
          patternstr+=GetUTFOctets(patternstr);
        if (GetUTFCode(patternstr) != 0)
          patternstr+=GetUTFOctets(patternstr);
        target=DestroyString(target);
        break;
      }
      case '\\':
      {
        patternstr+=GetUTFOctets(patternstr);
        if (GetUTFCode(patternstr) == 0)
          break;
      }
      default:
      {
        if (case_insensitive != MagickFalse)
          {
            if (tolower((int) GetUTFCode(expression)) !=
                tolower((int) GetUTFCode(patternstr)))
              {
                done=MagickTrue;
                break;
              }
          }
        else
          if (GetUTFCode(expression) != GetUTFCode(patternstr))
            {
              done=MagickTrue;
              break;
            }
        expression+=GetUTFOctets(expression);
        patternstr+=GetUTFOctets(patternstr);
      }
    }
  }
  while (GetUTFCode(patternstr) == '*')
    patternstr+=GetUTFOctets(patternstr);
  match=(GetUTFCode(expression) == 0) && (GetUTFCode(patternstr) == 0) ?
    MagickTrue : MagickFalse;
  return(match);
}