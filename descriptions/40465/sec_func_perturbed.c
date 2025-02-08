static inline char *
xmemdup0 (const char *in, size_t length)
{
  return xmemdup (in, length, length + 1);
}