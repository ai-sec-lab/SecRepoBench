static inline mrb_int
memsearch_swar(const char *needle, long m, const char *ys, long n)
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

  // <MASK>

  if (i+m < n) {
    const char *p = s0;
    const char *e = ys + n;
    for (;p<e;) {
      size_t len = e-p;
      p = (const char*)memchr(p, *needle, len);
      if (p == NULL || len < m) break;
      if (memcmp(p+1, needle+1, m-1) == 0) return (mrb_int)(p - ys);
      p++;
    }
  }

  return -1;
}