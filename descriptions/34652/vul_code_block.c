
      fmt = 'f';
      dec = -1;
      *s++ = first_dig;

      if (prec + e + 1 > buf_remaining) {
        prec = buf_remaining - e - 1;
      }

      if (org_fmt == 'g') {
        prec += (e - 1);
      }
      num_digits = prec;
      if (num_digits || alt_form) {
        *s++ = '.';
        while (--e && num_digits) {
          *s++ = '0';
          num_digits--;
        }
      }
    