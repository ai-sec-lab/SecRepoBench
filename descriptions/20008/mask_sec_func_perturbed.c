static double FxEvaluateSubexpression(FxInfo *fx_info,
  const PixelChannel channel,const ssize_t x,const ssize_t y,
  const char *result,const size_t depth,double *beta,
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
  q=(char *) result;
  alpha=InterpretSiPrefixValue(result,&q);
  if (q == result)
    alpha=FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception);
  FxReturn(alpha);
}