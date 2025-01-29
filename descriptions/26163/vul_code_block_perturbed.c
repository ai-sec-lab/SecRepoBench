if (k >= FP_SIZE)
          return FP_VAL;

      fpnumber->dp[k] |= ((fp_digit)ch) << j;
      j += 4;
      k += j == DIGIT_BIT;
      j &= DIGIT_BIT - 1;