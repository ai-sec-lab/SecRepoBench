static inline MagickBooleanType IsFxFunction(const char *expressionstr,
  const char *name,const size_t length)
{
  int
    c;

  c=name[length];
  if (c == '\0')
    return(MagickFalse);
  if ((LocaleNCompare(expressionstr,name,length) == 0) &&
      ((isspace(c) == 0) || (c == '(')))
    return(MagickTrue);
  return(MagickFalse);
}