while (1) {
      if (*rs == '.') {
        rs--;
        continue;
      }
      if (*rs < '0' || *rs > '9') {
        // + or -
        rs++; // So we sit on the digit to the right of the sign
        break;
      }
      if (*rs < '9') {
        (*rs)++;
        break;
      }
      *rs = '0';
      if (rs == buf) {
        break;
      }
      rs--;
    }
    if (*rs == '0') {
      // We need to insert a 1
      if (fmt != 'f' && rs[1] == '.') {
        // We're going to round 9.99 to 10.00
        // Move the decimal point
        rs[0] = '.';
        rs[1] = '0';
        if (e_sign == '-') {
          e--;
        }
        else {
          e++;
        }
      }
      s++;
      char *ss = s;
      while (ss > rs) {
        *ss = ss[-1];
        ss--;
      }
      *rs = '1';
      if (f < 1.0 && fmt == 'f') {
        // We rounded up to 1.0
        prec--;
      }
    }