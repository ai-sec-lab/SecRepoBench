int AffixMgr::cpdcase_check(const char* word, int index) {
  if (utf8) {
    const char* p;
    for (p = word + index - 1; p > word && (*p & 0xc0) == 0x80; p--)
      ;
    std::string pair(p);
    std::vector<w_char> pair_u;
    u8_u16(pair_u, pair);
    unsigned short a = pair_u.size() > 1 ? ((pair_u[1].h << 8) + pair_u[1].l) : 0;
    unsigned short b = !pair_u.empty() ? ((pair_u[0].h << 8) + pair_u[0].l) : 0;
    if (((unicodetoupper(a, langnum) == a) ||
         (unicodetoupper(b, langnum) == b)) &&
        (a != '-') && (b != '-'))
      return 1;
  } else {
    unsigned char a = *(word + index - 1);
    unsigned char b = *(word + index);
    if ((csconv[a].ccase || csconv[b].ccase) && (a != '-') && (b != '-'))
      return 1;
  }
  return 0;
}