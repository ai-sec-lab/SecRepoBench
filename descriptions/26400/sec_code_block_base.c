if (sym == 0) goto outofsym;
  if (SYMBOL_INLINE_P(sym)) return sym_inline_unpack(sym, buf, lenp);

#ifndef MRB_NO_PRESYM
  {
    const char *name = presym_sym2name(sym, lenp);
    if (name) return name;
  }
#endif
  sym -= MRB_PRESYM_MAX;

  if (mrb->symidx < sym) {
  outofsym:
    if (lenp) *lenp = 0;
    return NULL;
  }