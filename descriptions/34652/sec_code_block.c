
      fmt = 'f';
      dec = -1;
      *s++ = first_dig;

      if (org_fmt == 'g') {
        prec += (e - 1);
      }
      // truncate precision to prevent buffer overflow
      if (prec + 2 > buf_remaining) {
        prec = buf_remaining - 2;
      }
      num_digits = prec;
      if (num_digits || alt_form) {
        *s++ = '.';
        while (--e && num_digits) {
          *s++ = '0';
          num_digits--;
        }
      }
    