static double
  FxEvaluateSubexpression(FxInfo *,const PixelChannel,const ssize_t,
    const ssize_t,const char *,const size_t,double *,ExceptionInfo *);

static inline MagickBooleanType IsFxFunction(const char *expression,
  const char *name,const size_t length)
{
  int
    c;

  c=name[length];
  if ((LocaleNCompare(expression,name,length) == 0) &&
      ((isspace(c) == 0) || (c == '(')))
    return(MagickTrue);
  return(MagickFalse);
}