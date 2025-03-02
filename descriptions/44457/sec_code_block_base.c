if (pfx->usedOprStack == 0)
    return MagickFalse;
  if (pfx->OperatorStack[pfx->usedOprStack-1] == oQuery) {
    if (ptern->addrQuery != NULL_ADDRESS) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Already have '?' in sub-expression at", "'%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }
    if (ptern->addrColon != NULL_ADDRESS) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Already have ':' in sub-expression at", "'%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }
    pfx->usedOprStack--;
    ptern->addrQuery = pfx->usedElements;
    (void) AddAddressingElement (pfx, rIfZeroGoto, NULL_ADDRESS);
    /* address will be one after the Colon address. */
  }
  else if (pfx->OperatorStack[pfx->usedOprStack-1] == oColon) {
    if (ptern->addrQuery == NULL_ADDRESS) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Need '?' in sub-expression at", "'%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }
    if (ptern->addrColon != NULL_ADDRESS) {
      (void) ThrowMagickException (
        pfx->exception, GetMagickModule(), OptionError,
        "Already have ':' in sub-expression at", "'%s'",
        SetShortExp(pfx));
      return MagickFalse;
    }
    pfx->usedOprStack--;
    ptern->addrColon = pfx->usedElements;
    pfx->Elements[pfx->usedElements-1].DoPush = MagickTrue;
    (void) AddAddressingElement (pfx, rGoto, NULL_ADDRESS);
    /* address will be after the subexpression */
  }