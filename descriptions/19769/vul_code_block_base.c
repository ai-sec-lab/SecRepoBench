int
    c;

  c=name[length];
  if ((LocaleNCompare(expression,name,length) == 0) &&
      ((isspace(c) == 0) || (c == '(')))
    return(MagickTrue);
  return(MagickFalse);