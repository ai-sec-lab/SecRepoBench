static inline mrb_int
mrb_memsearch_ss(const unsigned char *xs, long m, const unsigned char *ys, long n)
{
#ifdef MRB_64BIT
#define bitint uint64_t
#define MASK1 0x0101010101010101ull
#define MASK2 0x7f7f7f7f7f7f7f7full
#define MASK3 0x8080808080808080ull
#else
#define bitint uint32_t
#define MASK1 0x01010101ul
#define MASK2 0x7f7f7f7ful
#define MASK3 0x80808080ul
#endif
#if defined(MRB_ENDIAN_BIG)
#ifdef MRB_64BIT
#define MASK4 0x8000000000000000ull
#else
#define MASK4 0x80000000ul
#endif
#else
#define MASK4 0x80
#endif

#define MAX(a,b) ((a) > (b) ? (a) : (b))

  const bitint first = MASK1 * (uint8_t)xs[0];
  const bitint last  = MASK1 * (uint8_t)xs[m-1];

  const unsigned char *s0 = ys;
  const unsigned char *s1 = ys+m-1;

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
        const unsigned char* p = s0 + idx + 1;
        if (memcmp(p, xs + 1, m - 2) == 0) {
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

  if (i+m < n) {
    const unsigned char *p = s0;
    const unsigned char *e = ys + n;
    for (;p<e;) {
      p = (const unsigned char*)memchr(p, *xs, e-p);
      if (p == NULL) break;
      if (memcmp(p+1, xs+1, m-1) == 0) return (mrb_int)(p - ys);
      p++;
    }
  }

  return -1;
}