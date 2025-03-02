if (strchr("(",(int) *expression) != (char *) NULL)
    {
      (*depth)++;
      if (*depth >= FxMaxParenthesisDepth)
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "ParenthesisNestedTooDeeply","`%s'",expression);
      (void) CopyMagickString(subexpression,expression+1,MagickPathExtent);
      subexpression[strlen(subexpression)-1]='\0';
      gamma=FxEvaluateSubexpression(fx_info,channel,x,y,subexpression,depth,
        beta,exception);
      (*depth)--;
      FxResult(gamma);
    }