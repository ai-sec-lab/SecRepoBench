case 1:
        ndx1 = pfx->usedElements;
        if (fe==fWhile) {
          (void) AddAddressingElement (pfx, rIfZeroGoto, NULL_ADDRESS); /* address will be ndx2+1 */
        } else if (fe==fDo) {
          (void) AddAddressingElement (pfx, rIfZeroGoto, NULL_ADDRESS); /* address will be ndx2+1 */
        } else if (fe==fFor) {
          pfx->Elements[pfx->usedElements-1].DoPush = MagickFalse;
        } else if (fe==fIf) {
          (void) AddAddressingElement (pfx, rIfZeroGoto, NULL_ADDRESS); /* address will be ndx2 + 1 */
          pfx->Elements[pfx->usedElements-1].DoPush = MagickTrue; /* we may need return from if() */
        }
        break;
      case 2:
        ndx2 = pfx->usedElements;
        if (fe==fWhile) {
          pfx->Elements[pfx->usedElements-1].DoPush = MagickFalse;
          (void) AddAddressingElement (pfx, rGoto, ndx0);
        } else if (fe==fDo) {
          pfx->Elements[pfx->usedElements-1].DoPush = MagickFalse;
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
        if (fe==fFor) {
          pfx->Elements[pfx->usedElements-1].DoPush = MagickFalse;
          (void) AddAddressingElement (pfx, rGoto, ndx1);
        }
        ndx3 = pfx->usedElements;
        break;