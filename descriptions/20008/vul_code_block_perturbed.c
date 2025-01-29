
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
  subexpression=AcquireString(result);
  *subexpression='\0';
  if (depth > FxMaxSubexpressionDepth)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "UnableToParseExpression","`%s'",result);
      FxReturn(0.0);
    }
  if (exception->severity >= ErrorException)
    FxReturn(0.0);
  while (isspace((int) ((unsigned char) *result)) != 0)
    result++;
  if (*result == '\0')
    FxReturn(0.0);
  p=FxOperatorPrecedence(result,exception);
  if (p != (const char *) NULL)
    {
      (void) CopyMagickString(subexpression,result,(size_t)
        (p-result+1));
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
  if (strchr("(",(int) *result) != (char *) NULL)
    {
      size_t
        length;

      if (depth >= FxMaxParenthesisDepth)
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "ParenthesisNestedTooDeeply","`%s'",result);
      length=CopyMagickString(subexpression,result+1,MagickPathExtent);
      if (length != 0)
        subexpression[length-1]='\0';
      gamma=FxEvaluateSubexpression(fx_info,channel,x,y,subexpression,depth+1,
        beta,exception);
      FxReturn(gamma);
    }
  switch (*result)
  {
    case '+':
    {
      gamma=FxEvaluateSubexpression(fx_info,channel,x,y,result+1,depth+1,
        beta,exception);
      FxReturn(1.0*gamma);
    }
    case '-':
    {
      gamma=FxEvaluateSubexpression(fx_info,channel,x,y,result+1,depth+1,
        beta,exception);
      FxReturn(-1.0*gamma);
    }
    case '~':
    {
      gamma=FxEvaluateSubexpression(fx_info,channel,x,y,result+1,depth+1,
        beta,exception);
      FxReturn((double) (~(size_t) (gamma+0.5)));
    }
    case 'A':
    case 'a':
    {
      if (IsFxFunction(result,"abs",3) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+3,
            depth+1,beta,exception);
          FxReturn(fabs(alpha));
        }
#if defined(MAGICKCORE_HAVE_ACOSH)
      if (IsFxFunction(result,"acosh",5) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+5,
            depth+1,beta,exception);
          FxReturn(acosh(alpha));
        }
#endif
      if (IsFxFunction(result,"acos",4) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+4,
            depth+1,beta,exception);
          FxReturn(acos(alpha));
        }
#if defined(MAGICKCORE_HAVE_J1)
      if (IsFxFunction(result,"airy",4) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+4,
            depth+1,beta,exception);
          if (alpha == 0.0)
            FxReturn(1.0);
          gamma=2.0*j1((MagickPI*alpha))/(MagickPI*alpha);
          FxReturn(gamma*gamma);
        }
#endif
#if defined(MAGICKCORE_HAVE_ASINH)
      if (IsFxFunction(result,"asinh",5) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+5,
            depth+1,beta,exception);
          FxReturn(asinh(alpha));
        }
#endif
      if (IsFxFunction(result,"asin",4) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+4,
            depth+1,beta,exception);
          FxReturn(asin(alpha));
        }
      if (IsFxFunction(result,"alt",3) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+3,
            depth+1,beta,exception);
          FxReturn(((ssize_t) alpha) & 0x01 ? -1.0 : 1.0);
        }
      if (IsFxFunction(result,"atan2",5) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+5,
            depth+1,beta,exception);
          FxReturn(atan2(alpha,*beta));
        }
#if defined(MAGICKCORE_HAVE_ATANH)
      if (IsFxFunction(result,"atanh",5) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+5,
            depth+1,beta,exception);
          FxReturn(atanh(alpha));
        }
#endif
      if (IsFxFunction(result,"atan",4) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+4,
            depth+1,beta,exception);
          FxReturn(atan(alpha));
        }
      if (LocaleCompare(result,"a") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      break;
    }
    case 'B':
    case 'b':
    {
      if (LocaleCompare(result,"b") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      break;
    }
    case 'C':
    case 'c':
    {
      if (IsFxFunction(result,"ceil",4) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+4,
            depth+1,beta,exception);
          FxReturn(ceil(alpha));
        }
      if (IsFxFunction(result,"clamp",5) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+5,
            depth+1,beta,exception);
          if (alpha < 0.0)
            FxReturn(0.0);
          if (alpha > 1.0)
            FxReturn(1.0);
          FxReturn(alpha);
        }
      if (IsFxFunction(result,"cosh",4) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+4,
            depth+1,beta,exception);
          FxReturn(cosh(alpha));
        }
      if (IsFxFunction(result,"cos",3) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+3,
            depth+1,beta,exception);
          FxReturn(cos(alpha));
        }
      if (LocaleCompare(result,"c") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      break;
    }
    case 'D':
    case 'd':
    {
      if (IsFxFunction(result,"debug",5) != MagickFalse)
        {
          const char
            *type;

          size_t
            length;

          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+5,
            depth+1,beta,exception);
          switch (fx_info->images->colorspace)
          {
            case CMYKColorspace:
            {
              switch (channel)
              {
                case CyanPixelChannel: type="cyan"; break;
                case MagentaPixelChannel: type="magenta"; break;
                case YellowPixelChannel: type="yellow"; break;
                case AlphaPixelChannel: type="alpha"; break;
                case BlackPixelChannel: type="black"; break;
                default: type="unknown"; break;
              }
              break;
            }
            case GRAYColorspace:
            {
              switch (channel)
              {
                case RedPixelChannel: type="gray"; break;
                case AlphaPixelChannel: type="alpha"; break;
                default: type="unknown"; break;
              }
              break;
            }
            default:
            {
              switch (channel)
              {
                case RedPixelChannel: type="red"; break;
                case GreenPixelChannel: type="green"; break;
                case BluePixelChannel: type="blue"; break;
                case AlphaPixelChannel: type="alpha"; break;
                default: type="unknown"; break;
              }
              break;
            }
          }
          *subexpression='\0';
          length=1;
          if (strlen(result) > 6)
            length=CopyMagickString(subexpression,result+6,
              MagickPathExtent);
          if (length != 0)
            subexpression[length-1]='\0';
          if (fx_info->file != (FILE *) NULL)
            (void) FormatLocaleFile(fx_info->file,"%s[%.20g,%.20g].%s: "
              "%s=%.*g\n",fx_info->images->filename,(double) x,(double) y,type,
              subexpression,GetMagickPrecision(),alpha);
          FxReturn(alpha);
        }
      if (IsFxFunction(result,"do",2) != MagickFalse)
        {
          size_t
            length;

          /*
            Parse do(expression,condition test).
          */
          length=CopyMagickString(subexpression,result+3,MagickPathExtent);
          if (length != 0)
            subexpression[length-1]='\0';
          p=subexpression;
          for (q=(char *) p; (*q != ',') && (*q != '\0'); q++)
            if (*q == '(')
              for ( ; (*q != ')') && (*q != '\0'); q++);
          if (*q == '\0')
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"UnableToParseExpression","`%s'",subexpression);
              FxReturn(0.0);
            }
          for (*q='\0'; ; )
          {
            alpha=FxEvaluateSubexpression(fx_info,channel,x,y,q+1,depth+1,beta,
              exception);
            gamma=FxEvaluateSubexpression(fx_info,channel,x,y,p,depth+1,&sans,
              exception);
            if (fabs(gamma) < MagickEpsilon)
              break;
          }
          FxReturn(alpha);
        }
      if (IsFxFunction(result,"drc",3) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+3,
            depth+1,beta,exception);
          FxReturn((alpha/(*beta*(alpha-1.0)+1.0)));
        }
      break;
    }
    case 'E':
    case 'e':
    {
      if (LocaleCompare(result,"epsilon") == 0)
        FxReturn(MagickEpsilon);
#if defined(MAGICKCORE_HAVE_ERF)
      if (IsFxFunction(result,"erf",3) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+3,
            depth+1,beta,exception);
          FxReturn(erf(alpha));
        }
#endif
      if (IsFxFunction(result,"exp",3) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+3,
            depth+1,beta,exception);
          FxReturn(exp(alpha));
        }
      if (LocaleCompare(result,"e") == 0)
        FxReturn(2.7182818284590452354);
      break;
    }
    case 'F':
    case 'f':
    {
      if (IsFxFunction(result,"floor",5) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+5,
            depth+1,beta,exception);
          FxReturn(floor(alpha));
        }
      if (IsFxFunction(result,"for",3) != MagickFalse)
        {
          double
            sans = 0.0;

          size_t
            length;

          /*
            Parse for(initialization, condition test, expression).
          */
          length=CopyMagickString(subexpression,result+4,MagickPathExtent);
          if (length != 0)
            subexpression[length-1]='\0';
          p=subexpression;
          for (q=(char *) p; (*q != ',') && (*q != '\0'); q++)
            if (*q == '(')
              for ( ; (*q != ')') && (*q != '\0'); q++);
          if (*q == '\0')
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"UnableToParseExpression","`%s'",subexpression);
              FxReturn(0.0);
            }
          *q='\0';
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,p,depth+1,&sans,
            exception);
          (void) CopyMagickString(subexpression,q+1,MagickPathExtent);
          p=subexpression;
          for (q=(char *) p; (*q != ',') && (*q != '\0'); q++)
            if (*q == '(')
              for ( ; (*q != ')') && (*q != '\0'); q++);
          if (*q == '\0')
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"UnableToParseExpression","`%s'",subexpression);
              FxReturn(0.0);
            }
          for (*q='\0'; ; )
          {
            gamma=FxEvaluateSubexpression(fx_info,channel,x,y,p,depth+1,&sans,
              exception);
            if (fabs(gamma) < MagickEpsilon)
              break;
            alpha=FxEvaluateSubexpression(fx_info,channel,x,y,q+1,depth+1,beta,
              exception);
          }
          FxReturn(alpha);
        }
      break;
    }
    case 'G':
    case 'g':
    {
      if (IsFxFunction(result,"gauss",5) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+5,
            depth+1,beta,exception);
          FxReturn(exp((-alpha*alpha/2.0))/sqrt(2.0*MagickPI));
        }
      if (IsFxFunction(result,"gcd",3) != MagickFalse)
        {
          MagickOffsetType
            gcd;

          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+3,
            depth+1,beta,exception);
          gcd=FxGCD((MagickOffsetType) (alpha+0.5),(MagickOffsetType) (*beta+
            0.5));
          FxReturn((double) gcd);
        }
      if (LocaleCompare(result,"g") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      break;
    }
    case 'H':
    case 'h':
    {
      if (LocaleCompare(result,"h") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      if (LocaleCompare(result,"hue") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      if (IsFxFunction(result,"hypot",5) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+5,
            depth+1,beta,exception);
          FxReturn(hypot(alpha,*beta));
        }
      break;
    }
    case 'K':
    case 'k':
    {
      if (LocaleCompare(result,"k") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      break;
    }
    case 'I':
    case 'i':
    {
      if (IsFxFunction(result,"if",2) != MagickFalse)
        {
          double
            sans = 0.0;

          size_t
            length;

          length=CopyMagickString(subexpression,result+3,MagickPathExtent);
          if (length != 0)
            subexpression[length-1]='\0';
          p=subexpression;
          for (q=(char *) p; (*q != ',') && (*q != '\0'); q++)
            if (*q == '(')
              for ( ; (*q != ')') && (*q != '\0'); q++);
          if (*q == '\0')
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"UnableToParseExpression","`%s'",subexpression);
              FxReturn(0.0);
            }
          *q='\0';
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,p,depth+1,&sans,
            exception);
          (void) CopyMagickString(subexpression,q+1,MagickPathExtent);
          p=subexpression;
          for (q=(char *) p; (*q != ',') && (*q != '\0'); q++)
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
            alpha=FxEvaluateSubexpression(fx_info,channel,x,y,p,depth+1,beta,
              exception);
          else
            alpha=FxEvaluateSubexpression(fx_info,channel,x,y,q+1,depth+1,beta,
              exception);
          FxReturn(alpha);
        }
      if (LocaleCompare(result,"intensity") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      if (IsFxFunction(result,"int",3) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+3,
            depth+1,beta,exception);
          FxReturn(floor(alpha));
        }
      if (IsFxFunction(result,"isnan",5) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+5,
            depth+1,beta,exception);
          FxReturn((double) !!IsNaN(alpha));
        }
      if (LocaleCompare(result,"i") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      break;
    }
    case 'J':
    case 'j':
    {
      if (LocaleCompare(result,"j") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
#if defined(MAGICKCORE_HAVE_J0)
      if (IsFxFunction(result,"j0",2) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+2,
            depth+1,beta,exception);
          FxReturn(j0(alpha));
        }
#endif
#if defined(MAGICKCORE_HAVE_J1)
      if (IsFxFunction(result,"j1",2) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+2,
            depth+1,beta,exception);
          FxReturn(j1(alpha));
        }
#endif
#if defined(MAGICKCORE_HAVE_J1)
      if (IsFxFunction(result,"jinc",4) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+4,
            depth+1,beta,exception);
          if (alpha == 0.0)
            FxReturn(1.0);
          FxReturn((2.0*j1((MagickPI*alpha))/(MagickPI*alpha)));
        }
#endif
      break;
    }
    case 'L':
    case 'l':
    {
      if (IsFxFunction(result,"ln",2) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+2,
            depth+1,beta,exception);
          FxReturn(log(alpha));
        }
      if (IsFxFunction(result,"logtwo",6) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+6,
            depth+1,beta,exception);
          FxReturn(log10(alpha)/log10(2.0));
        }
      if (IsFxFunction(result,"log",3) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+3,
            depth+1,beta,exception);
          FxReturn(log10(alpha));
        }
      if (LocaleCompare(result,"lightness") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      break;
    }
    case 'M':
    case 'm':
    {
      if (LocaleCompare(result,"MaxRGB") == 0)
        FxReturn(QuantumRange);
      if (LocaleNCompare(result,"maxima",6) == 0)
        break;
      if (IsFxFunction(result,"max",3) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+3,
            depth+1,beta,exception);
          FxReturn(alpha > *beta ? alpha : *beta);
        }
      if (LocaleNCompare(result,"minima",6) == 0)
        break;
      if (IsFxFunction(result,"min",3) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+3,
            depth+1,beta,exception);
          FxReturn(alpha < *beta ? alpha : *beta);
        }
      if (IsFxFunction(result,"mod",3) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+3,
            depth+1,beta,exception);
          FxReturn(alpha-floor((alpha*PerceptibleReciprocal(*beta)))*(*beta));
        }
      if (LocaleCompare(result,"m") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      break;
    }
    case 'N':
    case 'n':
    {
      if (IsFxFunction(result,"not",3) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+3,
            depth+1,beta,exception);
          FxReturn((double) (alpha < MagickEpsilon));
        }
      if (LocaleCompare(result,"n") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      break;
    }
    case 'O':
    case 'o':
    {
      if (LocaleCompare(result,"Opaque") == 0)
        FxReturn(1.0);
      if (LocaleCompare(result,"o") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      break;
    }
    case 'P':
    case 'p':
    {
      if (LocaleCompare(result,"phi") == 0)
        FxReturn(MagickPHI);
      if (LocaleCompare(result,"pi") == 0)
        FxReturn(MagickPI);
      if (IsFxFunction(result,"pow",3) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+3,
            depth+1,beta,exception);
          FxReturn(pow(alpha,*beta));
        }
      if (LocaleCompare(result,"p") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      break;
    }
    case 'Q':
    case 'q':
    {
      if (LocaleCompare(result,"QuantumRange") == 0)
        FxReturn(QuantumRange);
      if (LocaleCompare(result,"QuantumScale") == 0)
        FxReturn(QuantumScale);
      break;
    }
    case 'R':
    case 'r':
    {
      if (IsFxFunction(result,"rand",4) != MagickFalse)
        {
#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_FxEvaluateSubexpression)
#endif
          alpha=GetPseudoRandomValue(fx_info->random_info);
          FxReturn(alpha);
        }
      if (IsFxFunction(result,"round",5) != MagickFalse)
        {
          /*
            Round the fraction to nearest integer.
          */
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+5,
            depth+1,beta,exception);
          if ((alpha-floor(alpha)) < (ceil(alpha)-alpha))
            FxReturn(floor(alpha));
          FxReturn(ceil(alpha));
        }
      if (LocaleCompare(result,"r") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      break;
    }
    case 'S':
    case 's':
    {
      if (LocaleCompare(result,"saturation") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      if (IsFxFunction(result,"sign",4) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+4,
            depth+1,beta,exception);
          FxReturn(alpha < 0.0 ? -1.0 : 1.0);
        }
      if (IsFxFunction(result,"sinc",4) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+4,
            depth+1,beta,exception);
          if (alpha == 0)
            FxReturn(1.0);
          FxReturn(sin((MagickPI*alpha))/(MagickPI*alpha));
        }
      if (IsFxFunction(result,"sinh",4) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+4,
            depth+1,beta,exception);
          FxReturn(sinh(alpha));
        }
      if (IsFxFunction(result,"sin",3) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+3,
            depth+1,beta,exception);
          FxReturn(sin(alpha));
        }
      if (IsFxFunction(result,"sqrt",4) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+4,
            depth+1,beta,exception);
          FxReturn(sqrt(alpha));
        }
      if (IsFxFunction(result,"squish",6) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+6,
            depth+1,beta,exception);
          FxReturn((1.0/(1.0+exp(-alpha))));
        }
      if (LocaleCompare(result,"s") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      break;
    }
    case 'T':
    case 't':
    {
      if (IsFxFunction(result,"tanh",4) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+4,
            depth+1,beta,exception);
          FxReturn(tanh(alpha));
        }
      if (IsFxFunction(result,"tan",3) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+3,
            depth+1,beta,exception);
          FxReturn(tan(alpha));
        }
      if (LocaleCompare(result,"Transparent") == 0)
        FxReturn(0.0);
      if (IsFxFunction(result,"trunc",5) != MagickFalse)
        {
          alpha=FxEvaluateSubexpression(fx_info,channel,x,y,result+5,
            depth+1,beta,exception);
          if (alpha >= 0.0)
            FxReturn(floor(alpha));
          FxReturn(ceil(alpha));
        }
      if (LocaleCompare(result,"t") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      break;
    }
    case 'U':
    case 'u':
    {
      if (LocaleCompare(result,"u") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      break;
    }
    case 'V':
    case 'v':
    {
      if (LocaleCompare(result,"v") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      break;
    }
    case 'W':
    case 'w':
    {
      if (IsFxFunction(result,"while",5) != MagickFalse)
        {
          size_t
            length;

          /*
            Parse while(condition test, expression).
          */
          length=CopyMagickString(subexpression,result+6,MagickPathExtent);
          if (length != 0)
            subexpression[length-1]='\0';
          p=subexpression;
          for (q=(char *) p; (*q != ',') && (*q != '\0'); q++)
            if (*q == '(')
              for ( ; (*q != ')') && (*q != '\0'); q++);
          if (*q == '\0')
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"UnableToParseExpression","`%s'",subexpression);
              FxReturn(0.0);
            }
          for (*q='\0'; ; )
          {
            gamma=FxEvaluateSubexpression(fx_info,channel,x,y,p,depth+1,&sans,
              exception);
            if (fabs(gamma) < MagickEpsilon)
              break;
            alpha=FxEvaluateSubexpression(fx_info,channel,x,y,q+1,depth+1,beta,
              exception);
          }
          FxReturn(alpha);
        }
      if (LocaleCompare(result,"w") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      break;
    }
    case 'Y':
    case 'y':
    {
      if (LocaleCompare(result,"y") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      break;
    }
    case 'Z':
    case 'z':
    {
      if (LocaleCompare(result,"z") == 0)
        FxReturn(FxGetSymbol(fx_info,channel,x,y,result,depth+1,exception));
      break;
    }
    default:
      break;
  }