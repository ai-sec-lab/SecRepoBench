#define MAX(a,b) ((a) > (b) ? (a) : (b))

  const bitint first = MASK1 * (uint8_t)needle[0];
  const bitint last  = MASK1 * (uint8_t)needle[m-1];

  const char *s0 = ys;
  const char *s1 = ys+m-1;

  const mrb_int lim = n - MAX(m, (mrb_int)sizeof(bitint));
  mrb_int i;

  for (i=0; i < lim; i+=sizeof(bitint)) {
    bitint t0, t1;

    memcpy(&t0, s0+i, sizeof(bitint));
    memcpy(&t1, s1+i, sizeof(bitint));

    const bitint eq = (t0 ^ first) | (t1 ^ last);
    bitint zeros = ((~eq & MASK2) + MASK1) & (~eq & MASK3);
    size_t j = 0;

    while (zeros) {
      if (zeros & MASK4) {
        const mrb_int idx = i + j;
        const char* p = s0 + idx + 1;
        if (memcmp(p, needle + 1, m - 2) == 0) {
          return idx;
        }
      }

#if defined(MRB_ENDIAN_BIG)
      zeros <<= 8;
#else
      zeros >>= 8;
#endif
      j++;
    }
  }