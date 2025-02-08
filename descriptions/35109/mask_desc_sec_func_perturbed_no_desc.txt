static const char *
get_num(mrb_state *mrb, const char *p, const char *end, int *valp)
{
  mrb_int nextnumber = (int)*valp;
  // <MASK>
  *valp = (int)nextnumber;
  return p;
}