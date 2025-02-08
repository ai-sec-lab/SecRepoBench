char *
bit_utf8_to_TV (char *restrict outputbuffer, const unsigned char *restrict src,
                const int destlen, const int srclen, const unsigned cquoted)
{
  unsigned char c;
  unsigned char *s = (unsigned char *)src;
  const char* endp = outputbuffer + destlen;
  const unsigned char* ends = src + srclen;
  char *d = outputbuffer;

  while ((c = *s++))
    {
      // <MASK>
      if (s >= ends)
        break;
      /* everything above 0xf0 exceeds ucs-2, 4-6 byte seqs */
    }

  if (outputbuffer >= endp)
    return NULL;
  else
    *outputbuffer = '\0';
  return d;
}