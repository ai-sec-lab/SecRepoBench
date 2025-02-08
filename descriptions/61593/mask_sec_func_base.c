static ATTRIBUTE_MALLOC char *
bit_TV_to_utf8_codepage (const char *restrict src, const BITCODE_RS codepage)
{
  const bool is_asian_cp = dwg_codepage_isasian ((const Dwg_Codepage)codepage);
  const size_t srclen = strlen (src);
  size_t destlen = is_asian_cp ? srclen * 3 : trunc (srclen * 1.5);
  size_t i = 0;
  char *str = (char *)calloc (1, destlen + 1);
  unsigned char *tmp = (unsigned char *)src;
  uint16_t c = 0;

  if (!srclen)
    return (char *)calloc (1, 1);
  // <MASK>
  EXTEND_SIZE (str, i + 1, destlen);
  str[i] = '\0';
  return bit_u_expand (str);
}