static inline char *
xmemdup0 (const char *in, size_t len)
{
  return xmemdup (in, len, len + 1);
}