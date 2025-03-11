case 1:
        if (ndx1 != NULL_ADDRESS) {
          (void) ThrowMagickException (
            pfx->exception, GetMagickModule(), OptionError,
            "For function", "'%s' required argument is missing at '%s'",
            funStr, SetShortExp(pfx));
          return isCoordQualifier;
        }
        ndx1 = pfx->usedElements;
        if (fe==fWhile) {
          (void) AddAddressingElement (pfx, rIfZeroGoto, NULL_ADDRESS); /* address will be ndx2+1 */
        } else if (fe==fDo) {
          (void) AddAddressingElement (pfx, rIfZeroGoto, NULL_ADDRESS); /* address will be ndx2+1 */
        } else if (fe==fFor) {
          pfx->Elements[pfx->usedElements-1].DoPush = isCoordQualifier;
        } else if (fe==fIf) {
          (void) AddAddressingElement (pfx, rIfZeroGoto, NULL_ADDRESS); /* address will be ndx2 + 1 */
          pfx->Elements[pfx->usedElements-1].DoPush = MagickTrue; /* we may need return from if() */
        }
        break;
      case 2:
        if (ndx2 != NULL_ADDRESS) {
          (void) ThrowMagickException (
            pfx->exception, GetMagickModule(), OptionError,
            "For function", "'%s' required argument is missing at '%s'",
            funStr, SetShortExp(pfx));
          return isCoordQualifier;
        }
        ndx2 = pfx->usedElements;
        if (fe==fWhile) {
          pfx->Elements[pfx->usedElements-1].DoPush = isCoordQualifier;
          (void) AddAddressingElement (pfx, rGoto, ndx0);
        } else if (fe==fDo) {
          pfx->Elements[pfx->usedElements-1].DoPush = isCoordQualifier;
          (void) AddAddressingElement (pfx, rGoto, ndx0 + 1);
        } else if (fe==fFor) {
          (void) AddAddressingElement (pfx, rIfZeroGoto, NULL_ADDRESS); /* address will be ndx3 */
          pfx->Elements[pfx->usedElements-1].DoPush = MagickTrue; /* we may need return from for() */
          (void) AddAddressingElement (pfx, rZerStk, NULL_ADDRESS);
        } else if (fe==fIf) {
          (void) AddAddressingElement (pfx, rGoto, NULL_ADDRESS); /* address will be ndx3 */
        }
        break;
      case 3:
        if (ndx3 != NULL_ADDRESS) {
          (void) ThrowMagickException (
            pfx->exception, GetMagickModule(), OptionError,
            "For function", "'%s' required argument is missing at '%s'",
            funStr, SetShortExp(pfx));
          return isCoordQualifier;
        }
        if (fe==fFor) {
          pfx->Elements[pfx->usedElements-1].DoPush = isCoordQualifier;
          (void) AddAddressingElement (pfx, rGoto, ndx1);
        }
        ndx3 = pfx->usedElements;
        break;