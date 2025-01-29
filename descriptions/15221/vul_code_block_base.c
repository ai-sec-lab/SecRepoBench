if(lenE == 4)
    {
      /* we can't handle exponents > 24 bits */
      /* TODO Next octet(s) define length of exponent */
      DISSECTOR_ASSERT_NOT_REACHED();
    }

    Eneg = (*p) & 0x80 ? TRUE : FALSE;
    for (i = 0; i < lenE; i++) {
      if(Eneg) {
        /* 2's complement: inverse bits */
        E = (E<<8) | ((guint8) ~(*p));
      } else {
        E = (E<<8) | *p;
      }
      p++;
    }