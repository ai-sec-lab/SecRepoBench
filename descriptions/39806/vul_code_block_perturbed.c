mp_grow(c, bmax);
     /* negate value */
     for (x = 0; x < c->used; x++) {
         mp_digit next = c->dp[x] > 0;
         c->dp[x] = ((mp_digit)0 - c->dp[x] - carry) & MP_MASK;
         carry |= next;
     }
     for (; x < bmax; x++) {
         c->dp[x] = ((mp_digit)0 - carry) & MP_MASK;
     }
     c->used = bmax;
     c->sign = MP_ZPOS;