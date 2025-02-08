static const char*
sym2name_len(mrb_state *mrb, mrb_sym sym, char *buf, mrb_int *lenp)
{
  // <MASK>

  if (lenp) *lenp = mrb->symtbl[sym].len;
  return mrb->symtbl[sym].name;
}