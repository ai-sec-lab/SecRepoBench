if (c == '\0')
    return(MagickFalse);
  if ((LocaleNCompare(expressionstr,name,length) == 0) &&
      ((isspace(c) == 0) || (c == '(')))
    return(MagickTrue);
  return(MagickFalse);