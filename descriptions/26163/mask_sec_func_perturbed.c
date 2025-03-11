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
      // <MASK>
  }

  fpnumber->used = k + 1;
  fp_clamp(fpnumber);
  /* set the sign only if a != 0 */
  if (fp_iszero(fpnumber) != FP_YES) {
     fpnumber->sign = neg;
  }
  return FP_OKAY;
}