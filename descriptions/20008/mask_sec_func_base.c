static double FxEvaluateSubexpression(FxInfo *fx_info,
  const PixelChannel channel,const ssize_t x,const ssize_t y,
  const char *expression,const size_t depth,double *beta,
  ExceptionInfo *exception)
{
#define FxMaxParenthesisDepth  58
#define FxMaxSubexpressionDepth  200
#define FxReturn(value) \
{ \
  subexpression=DestroyString(subexpression); \
  return(value); \
}
// <MASK>
  subexpression=DestroyString(subexpression);
  q=(char *) expression;
  alpha=InterpretSiPrefixValue(expression,&q);
  if (q == expression)
    alpha=FxGetSymbol(fx_info,channel,x,y,expression,depth+1,exception);
  FxReturn(alpha);
}