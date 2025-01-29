for (int j = 0; j < 6; j++) {
      const CFAColor c = mRaw->cfa.getColorAt(j, i);
      switch (c) {
      case CFA_RED:
      case CFA_GREEN:
      case CFA_BLUE:
        CFA[i][j] = c;
        break;
      default:
        ThrowRDE("Got unexpected color %u", c);
      }
    }