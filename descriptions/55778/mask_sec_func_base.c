int fp_to_unsigned_bin_len(fp_int *a, unsigned char *b, int c)
{
#if DIGIT_BIT == 64 || DIGIT_BIT == 32 || DIGIT_BIT == 16
  int i = 0;
  int j = 0;
  int x;

  for (x=c-1; x >= 0 && i < a->used; x--) {
     b[x] = (unsigned char)(a->dp[i] >> j);
     j += 8;
     i += j == DIGIT_BIT;
     j &= DIGIT_BIT - 1;
  }
  for (; x >= 0; x--) {
     b[x] = 0;
  }
  // <MASK>
}