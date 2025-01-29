GetNextToken(p,&p,MagickPathExtent,token);
          if (*token == ',')
            GetNextToken(p,&p,MagickPathExtent,token);
          sweep=fabs(StringToDouble(token&next_token)) < DrawEpsilon ?
            MagickFalse : MagickTrue;
          if (token == next_token)
            ThrowPointExpectedException(token,exception);