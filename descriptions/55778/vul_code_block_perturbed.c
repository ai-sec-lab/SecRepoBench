if ((i < a->used - 1) || ((a->dp[i] >> bitoffset) != 0)) {
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