if (fxinfo->OperatorStack[fxinfo->usedOprStack-1] == oQuery) {
    if (ptern->addrQuery != NULL_ADDRESS) {
      (void) ThrowMagickException (
        fxinfo->exception, GetMagickModule(), OptionError,
        "Already have '?' in sub-expression at", "'%s'",
        SetShortExp(fxinfo));
      return MagickFalse;
    }
    if (ptern->addrColon != NULL_ADDRESS) {
      (void) ThrowMagickException (
        fxinfo->exception, GetMagickModule(), OptionError,
        "Already have ':' in sub-expression at", "'%s'",
        SetShortExp(fxinfo));
      return MagickFalse;
    }
    fxinfo->usedOprStack--;
    ptern->addrQuery = fxinfo->usedElements;
    (void) AddAddressingElement (fxinfo, rIfZeroGoto, NULL_ADDRESS);
    /* address will be one after the Colon address. */
  }
  else if (fxinfo->OperatorStack[fxinfo->usedOprStack-1] == oColon) {
    if (ptern->addrQuery == NULL_ADDRESS) {
      (void) ThrowMagickException (
        fxinfo->exception, GetMagickModule(), OptionError,
        "Need '?' in sub-expression at", "'%s'",
        SetShortExp(fxinfo));
      return MagickFalse;
    }
    if (ptern->addrColon != NULL_ADDRESS) {
      (void) ThrowMagickException (
        fxinfo->exception, GetMagickModule(), OptionError,
        "Already have ':' in sub-expression at", "'%s'",
        SetShortExp(fxinfo));
      return MagickFalse;
    }
    fxinfo->usedOprStack--;
    ptern->addrColon = fxinfo->usedElements;
    fxinfo->Elements[fxinfo->usedElements-1].DoPush = MagickTrue;
    (void) AddAddressingElement (fxinfo, rGoto, NULL_ADDRESS);
    /* address will be after the subexpression */
  }