if (org_fmt == 'g') {
        prec += (e - 1);
      }
      // truncate precision to prevent buffer overflow
      if (prec + 2 > buf_remaining) {
        prec = buf_remaining - 2;
      }