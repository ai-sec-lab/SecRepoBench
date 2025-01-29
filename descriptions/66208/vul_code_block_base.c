  const bitint first = MASK1 * (uint8_t)xs[0];
  const bitint last  = MASK1 * (uint8_t)xs[m-1];

  bitint *s0 = (bitint*)(ys);
  bitint *s1 = (bitint*)(ys+m-1);

  for (mrb_int i=0; i < n; i+=sizeof(bitint), s0++, s1++) {
    const bitint eq = (*s0 ^ first) | (*s1 ^ last);
    bitint zeros = ((~eq & MASK2) + MASK1) & (~eq & MASK3);
    size_t j = 0;

    while (zeros) {
      if (zeros & MASK4) {
        const char* substr = (char*)s0 + j + 1;
        if (memcmp(substr, xs + 1, m - 2) == 0) {
          return i + j;
        }
      }

#if defined(MRB_ENDIAN_BIG)
      zeros <<= 8;
#else
      zeros >>= 8;
#endif
      j += 1;
    }
  }