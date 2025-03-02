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

  char
    *q,
    *subexpression;

  double
    alpha,
    gamma,
    sans,
    value;

  register const char
    *p;

  *beta=0.0;
  sans=0.0;
  subexpression=AcquireString(expression);
  *subexpression='\0';
  if (depth > FxMaxSubexpressionDepth)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "UnableToParseExpression","`%s'",expression);
      FxReturn(0.0);
    }
  if (exception->severity >= ErrorException)
    FxReturn(0.0);
  while (isspace((int) ((unsigned char) *expression)) != 0)
    expression++;
  if (*expression == '\0')
    FxReturn(0.0);
  p=FxOperatorPrecedence(expression,exception);
  if (p != (const char *) NULL)
    {
      (void) CopyMagickString(subexpression,expression,(size_t)
        (p-expression+1));
      alpha=FxEvaluateSubexpression(fx_info,channel,x,y,subexpression,depth+1,
        beta,exception);
      switch ((unsigned char) *p)
      {
        case '~':
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          *beta=(double) (~(size_t) *beta);
          FxReturn(*beta);
        }
        case '!':
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          FxReturn(*beta == 0.0 ? 1.0 : 0.0);
        }
        case '^':
        {
          *beta=pow(alpha,FxEvaluateSubexpression(fx_info,channel,x,y,++p,
            depth+1,beta,exception));
          FxReturn(*beta);
        }
        case '*':
        case ExponentialNotation:
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          FxReturn(alpha*(*beta));
        }
        case '/':
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          FxReturn(PerceptibleReciprocal(*beta)*alpha);
        }
        case '%':
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          FxReturn(fmod(alpha,*beta));
        }
        case '+':
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          FxReturn(alpha+(*beta));
        }
        case '-':
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          FxReturn(alpha-(*beta));
        }
        case BitwiseAndAssignmentOperator:
        {
          q=subexpression;
          while (isalpha((int) ((unsigned char) *q)) != 0)
            q++;
          if (*q != '\0')
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"UnableToParseExpression","`%s'",subexpression);
              FxReturn(0.0);
            }
          ClearMagickException(exception);
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          value=(double) ((size_t) (alpha+0.5) & (size_t) (*beta+0.5));
          if (SetFxSymbolValue(fx_info,subexpression,value) == MagickFalse)
            return(0.0);
          FxReturn(*beta);
        }
        case BitwiseOrAssignmentOperator:
        {
          q=subexpression;
          while (isalpha((int) ((unsigned char) *q)) != 0)
            q++;
          if (*q != '\0')
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"UnableToParseExpression","`%s'",subexpression);
              FxReturn(0.0);
            }
          ClearMagickException(exception);
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          value=(double) ((size_t) (alpha+0.5) | (size_t) (*beta+0.5));
          if (SetFxSymbolValue(fx_info,subexpression,value) == MagickFalse)
            return(0.0);
          FxReturn(*beta);
        }
        case LeftShiftAssignmentOperator:
        {
          q=subexpression;
          while (isalpha((int) ((unsigned char) *q)) != 0)
            q++;
          if (*q != '\0')
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"UnableToParseExpression","`%s'",subexpression);
              FxReturn(0.0);
            }
          ClearMagickException(exception);
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          if ((size_t) (*beta+0.5) >= (8*sizeof(size_t)))
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"ShiftCountOverflow","`%s'",subexpression);
              FxReturn(0.0);
            }
          value=(double) ((size_t) (alpha+0.5) << (size_t) (*beta+0.5));
          if (SetFxSymbolValue(fx_info,subexpression,value) == MagickFalse)
            return(0.0);
          FxReturn(*beta);
        }
        case RightShiftAssignmentOperator:
        {
          q=subexpression;
          while (isalpha((int) ((unsigned char) *q)) != 0)
            q++;
          if (*q != '\0')
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"UnableToParseExpression","`%s'",subexpression);
              FxReturn(0.0);
            }
          ClearMagickException(exception);
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          if ((size_t) (*beta+0.5) >= (8*sizeof(size_t)))
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"ShiftCountOverflow","`%s'",subexpression);
              FxReturn(0.0);
            }
          value=(double) ((size_t) (alpha+0.5) >> (size_t) (*beta+0.5));
          if (SetFxSymbolValue(fx_info,subexpression,value) == MagickFalse)
            return(0.0);
          FxReturn(*beta);
        }
        case PowerAssignmentOperator:
        {
          q=subexpression;
          while (isalpha((int) ((unsigned char) *q)) != 0)
            q++;
          if (*q != '\0')
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"UnableToParseExpression","`%s'",subexpression);
              FxReturn(0.0);
            }
          ClearMagickException(exception);
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          value=pow(alpha,*beta);
          if (SetFxSymbolValue(fx_info,subexpression,value) == MagickFalse)
            return(0.0);
          FxReturn(*beta);
        }
        case ModuloAssignmentOperator:
        {
          q=subexpression;
          while (isalpha((int) ((unsigned char) *q)) != 0)
            q++;
          if (*q != '\0')
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"UnableToParseExpression","`%s'",subexpression);
              FxReturn(0.0);
            }
          ClearMagickException(exception);
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          value=fmod(alpha,*beta);
          if (SetFxSymbolValue(fx_info,subexpression,value) == MagickFalse)
            return(0.0);
          FxReturn(*beta);
        }
        case PlusAssignmentOperator:
        {
          q=subexpression;
          while (isalpha((int) ((unsigned char) *q)) != 0)
            q++;
          if (*q != '\0')
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"UnableToParseExpression","`%s'",subexpression);
              FxReturn(0.0);
            }
          ClearMagickException(exception);
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          value=alpha+(*beta);
          if (SetFxSymbolValue(fx_info,subexpression,value) == MagickFalse)
            return(0.0);
          FxReturn(*beta);
        }
        case SubtractAssignmentOperator:
        {
          q=subexpression;
          while (isalpha((int) ((unsigned char) *q)) != 0)
            q++;
          if (*q != '\0')
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"UnableToParseExpression","`%s'",subexpression);
              FxReturn(0.0);
            }
          ClearMagickException(exception);
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          value=alpha-(*beta);
          if (SetFxSymbolValue(fx_info,subexpression,value) == MagickFalse)
            return(0.0);
          FxReturn(*beta);
        }
        case MultiplyAssignmentOperator:
        {
          q=subexpression;
          while (isalpha((int) ((unsigned char) *q)) != 0)
            q++;
          if (*q != '\0')
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"UnableToParseExpression","`%s'",subexpression);
              FxReturn(0.0);
            }
          ClearMagickException(exception);
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          value=alpha*(*beta);
          if (SetFxSymbolValue(fx_info,subexpression,value) == MagickFalse)
            return(0.0);
          FxReturn(*beta);
        }
        case DivideAssignmentOperator:
        {
          q=subexpression;
          while (isalpha((int) ((unsigned char) *q)) != 0)
            q++;
          if (*q != '\0')
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"UnableToParseExpression","`%s'",subexpression);
              FxReturn(0.0);
            }
          ClearMagickException(exception);
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          value=alpha*PerceptibleReciprocal(*beta);
          if (SetFxSymbolValue(fx_info,subexpression,value) == MagickFalse)
            return(0.0);
          FxReturn(*beta);
        }
        case IncrementAssignmentOperator:
        {
          if (*subexpression == '\0')
            alpha=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
              exception);
          value=alpha+1.0;
          if (*subexpression == '\0')
            {
              if (SetFxSymbolValue(fx_info,p,value) == MagickFalse)
                return(0.0);
            }
          else
            if (SetFxSymbolValue(fx_info,subexpression,value) == MagickFalse)
              return(0.0);
          FxReturn(*beta);
        }
        case DecrementAssignmentOperator:
        {
          if (*subexpression == '\0')
            alpha=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
              exception);
          value=alpha-1.0;
          if (*subexpression == '\0')
            {
              if (SetFxSymbolValue(fx_info,p,value) == MagickFalse)
                return(0.0);
            }
          else
            if (SetFxSymbolValue(fx_info,subexpression,value) == MagickFalse)
              return(0.0);
          FxReturn(*beta);
        }
        case LeftShiftOperator:
        {
          gamma=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          if ((size_t) (gamma+0.5) >= (8*sizeof(size_t)))
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"ShiftCountOverflow","`%s'",subexpression);
              FxReturn(0.0);
            }
          *beta=(double) ((size_t) (alpha+0.5) << (size_t) (gamma+0.5));
          FxReturn(*beta);
        }
        case RightShiftOperator:
        {
          gamma=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          if ((size_t) (gamma+0.5) >= (8*sizeof(size_t)))
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"ShiftCountOverflow","`%s'",subexpression);
              FxReturn(0.0);
            }
          *beta=(double) ((size_t) (alpha+0.5) >> (size_t) (gamma+0.5));
          FxReturn(*beta);
        }
        case '<':
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          FxReturn(alpha < *beta ? 1.0 : 0.0);
        }
        case LessThanEqualOperator:
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          FxReturn(alpha <= *beta ? 1.0 : 0.0);
        }
        case '>':
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          FxReturn(alpha > *beta ? 1.0 : 0.0);
        }
        case GreaterThanEqualOperator:
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          FxReturn(alpha >= *beta ? 1.0 : 0.0);
        }
        case EqualOperator:
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          FxReturn(fabs(alpha-(*beta)) < MagickEpsilon ? 1.0 : 0.0);
        }
        case NotEqualOperator:
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          FxReturn(fabs(alpha-(*beta)) >= MagickEpsilon ? 1.0 : 0.0);
        }
        case '&':
        {
          gamma=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          *beta=(double) ((size_t) (alpha+0.5) & (size_t) (gamma+0.5));
          FxReturn(*beta);
        }
        case '|':
        {
          gamma=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          *beta=(double) ((size_t) (alpha+0.5) | (size_t) (gamma+0.5));
          FxReturn(*beta);
        }
        case LogicalAndOperator:
        {
          p++;
          if (alpha <= 0.0)
            {
              *beta=0.0;
              FxReturn(*beta);
            }
          gamma=FxEvaluateSubexpression(fx_info,channel,x,y,p,depth+1,beta,
            exception);
          *beta=(gamma > 0.0) ? 1.0 : 0.0;
          FxReturn(*beta);
        }
        case LogicalOrOperator:
        {
          p++;
          if (alpha > 0.0)
            {
             *beta=1.0;
             FxReturn(*beta);
            }
          gamma=FxEvaluateSubexpression(fx_info,channel,x,y,p,depth+1,beta,
            exception);
          *beta=(gamma > 0.0) ? 1.0 : 0.0;
          FxReturn(*beta);
        }
        case '?':
        {
          (void) CopyMagickString(subexpression,++p,MagickPathExtent);
          p=subexpression;
          for (q=(char *) p; (*q != ':') && (*q != '\0'); q++)
            if (*q == '(')
              for ( ; (*q != ')') && (*q != '\0'); q++);
          if (*q == '\0')
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"UnableToParseExpression","`%s'",subexpression);
              FxReturn(0.0);
            }
          *q='\0';
          if (fabs(alpha) >= MagickEpsilon)
            gamma=FxEvaluateSubexpression(fx_info,channel,x,y,p,depth+1,beta,
              exception);
          else
            gamma=FxEvaluateSubexpression(fx_info,channel,x,y,q+1,depth+1,beta,
              exception);
          FxReturn(gamma);
        }
        case '=':
        {
          q=subexpression;
          while (isalpha((int) ((unsigned char) *q)) != 0)
            q++;
          if (*q != '\0')
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"UnableToParseExpression","`%s'",subexpression);
              FxReturn(0.0);
            }
          ClearMagickException(exception);
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          value=(*beta);
          if (SetFxSymbolValue(fx_info,subexpression,value) == MagickFalse)
            return(0.0);
          FxReturn(*beta);
        }
        case ',':
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          FxReturn(alpha);
        }
        case ';':
        {
          *beta=FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,beta,
            exception);
          FxReturn(*beta);
        }
        default:
        {
          gamma=alpha*FxEvaluateSubexpression(fx_info,channel,x,y,++p,depth+1,
            beta,exception);
          FxReturn(gamma);
        }
      }
    }
  // <MASK>
  subexpression=DestroyString(subexpression);
  q=(char *) expression;
  alpha=InterpretSiPrefixValue(expression,&q);
  if (q == expression)
    alpha=FxGetSymbol(fx_info,channel,x,y,expression,depth+1,exception);
  FxReturn(alpha);
}