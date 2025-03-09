      if (prec + e + 1 > buf_remaining) {
        prec = buf_remaining - e - 1;
      }

      if (originalformat == 'g') {
        prec += (e - 1);
      }