if (strchr("(",(int) *expression) != (char *) NULL)
    {
      (*recursion_depth)++;
      if (*recursion_depth >= FxMaxParenthesisDepth)
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "ParenthesisNestedTooDeeply","`%s'",expression);
      (void) CopyMagickString(subexpression,expression+1,MagickPathExtent);
      if (strlen(subexpression) != 0)
        subexpression[strlen(subexpression)-1]='\0';
      gamma=FxEvaluateSubexpression(fx_info,channel,x,y,subexpression,recursion_depth,
        beta,exception);
      (*recursion_depth)--;
      FxResult(gamma);
    }