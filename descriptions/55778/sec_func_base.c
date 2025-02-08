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
  if (i < a->used - 1) {
      return FP_VAL;
  }
  if ((i == a->used - 1) && ((a->dp[i] >> j) != 0)) {
      return FP_VAL;
  }

  return FP_OKAY;
#else
  int     x;
#ifndef WOLFSSL_SMALL_STACK
   fp_int t[1];
#else
   fp_int *t;
#endif

#ifdef WOLFSSL_SMALL_STACK
   t = (fp_int*)XMALLOC(sizeof(fp_int), NULL, DYNAMIC_TYPE_BIGINT);
   if (t == NULL)
       return FP_MEM;
#endif

  fp_init_copy(t, a);

  for (x = 0; x < c; x++) {
      b[x] = (unsigned char) (t->dp[0] & 255);
      fp_div_2d (t, 8, t, NULL);
  }
  mp_reverse (b, x);

#ifdef WOLFSSL_SMALL_STACK
  XFREE(t, NULL, DYNAMIC_TYPE_BIGINT);
#endif
  if (!fp_iszero(t)) {
      return FP_VAL;
  }
  return FP_OKAY;
#endif
}