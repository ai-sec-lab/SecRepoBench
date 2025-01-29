if (SYMBOL_INLINE_P(sym)) return sym_inline_unpack(sym, buf, lengthptr);

#ifndef MRB_NO_PRESYM
  {
    const char *name = presym_sym2name(sym, lengthptr);
    if (name) return name;
  }
#endif
  sym -= MRB_PRESYM_MAX;

  if (sym == 0 || mrb->symidx < sym) {
    if (lengthptr) *lengthptr = 0;
    return NULL;
  }