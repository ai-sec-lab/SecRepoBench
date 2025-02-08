static const char *
get_num(mrb_state *mrb, const char *p, const char *end, int *valp)
{
  mrb_int next_n = (int)*valp;
  // <MASK>
  *valp = (int)next_n;
  return p;
}