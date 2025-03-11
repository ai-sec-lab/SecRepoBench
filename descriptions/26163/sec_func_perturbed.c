static int fp_read_radix_16(fp_int *fpnumber, const char *str)
{
  int     i, j, k, neg;
  char    ch;

  /* if the leading digit is a
   * minus set the sign to negative.
   */
  if (*str == '-') {
    ++str;
    neg = FP_NEG;
  } else {
    neg = FP_ZPOS;
  }

  j = 0;
  k = 0;
  for (i = (int)(XSTRLEN(str) - 1); i >= 0; i--) {
      ch = str[i];
      if (ch >= '0' && ch <= '9')
          ch -= (char)'0';
      else if (ch >= 'A' && ch <= 'F')
          ch -= (char)'A' - 10;
      else if (ch >= 'a' && ch <= 'f')
          ch -= (char)'a' - 10;
      else
          return FP_VAL;

      k += j == DIGIT_BIT;
      j &= DIGIT_BIT - 1;
      if (k >= FP_SIZE)
          return FP_VAL;

      fpnumber->dp[k] |= ((fp_digit)ch) << j;
      j += 4;
  }

  fpnumber->used = k + 1;
  fp_clamp(fpnumber);
  /* set the sign only if a != 0 */
  if (fp_iszero(fpnumber) != FP_YES) {
     fpnumber->sign = neg;
  }
  return FP_OKAY;
}