char *
bit_TV_to_utf8 (const char *restrict src, const BITCODE_RS codepage)
{
  if (codepage == CP_UTF8)
    return bit_u_expand ((char *)src);
  else if (!src)
    return NULL;
  {
    const bool is_asian_cp
        = dwg_codepage_isasian ((const Dwg_Codepage)codepage);
    const size_t srclen = strlen (src);
    // <MASK>
  }
}