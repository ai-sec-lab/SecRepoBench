int mp_mod_2d (mp_int * a, int b, mp_int * c)
{
  int     x, res, bmax;

  /* if b is <= 0 then zero the int */
  if (b <= 0) {
    mp_zero (c);
    return MP_OKAY;
  }

  /* if the modulus is larger than the value than return */
  if (a->sign == MP_ZPOS && b >= (int) (a->used * DIGIT_BIT)) {
    res = mp_copy (a, c);
    return res;
  }

  /* copy */
  if ((res = mp_copy (a, c)) != MP_OKAY) {
    return res;
  }

  /* calculate number of digits in mod value */
  bmax = (b / DIGIT_BIT) + ((b % DIGIT_BIT) == 0 ? 0 : 1);
  /* zero digits above the last digit of the modulus */
  for (x = bmax; x < c->used; x++) {
    c->dp[x] = 0;
  }

  if (c->sign == MP_NEG) {
     mp_digit carry = 0;

     /* grow result to size of modulus */
     // <MASK>
  }

  /* clear the digit that is not completely outside/inside the modulus */
  x = DIGIT_BIT - (b % DIGIT_BIT);
  if (x != DIGIT_BIT) {
    c->dp[bmax - 1] &=
         ((mp_digit)~((mp_digit)0)) >> (x + ((sizeof(mp_digit)*8) - DIGIT_BIT));
  }
  mp_clamp (c);
  return MP_OKAY;
}