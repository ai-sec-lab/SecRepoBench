if (ch >= '0' && ch <= '9')
          ch -= (char)'0';
      else if (ch >= 'A' && ch <= 'F')
          ch -= (char)'A' - 10;
      else if (ch >= 'a' && ch <= 'f')
          ch -= (char)'a' - 10;
      else
          return FP_VAL;

      if (k >= FP_SIZE)
          return FP_VAL;

      fpnumber->dp[k] |= ((fp_digit)ch) << j;
      j += 4;
      k += j == DIGIT_BIT;
      j &= DIGIT_BIT - 1;