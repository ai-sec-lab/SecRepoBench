static const char *
get_num(mrb_state *mrb, const char *p, const char *limit, int *valp)
{
  mrb_int next_n = (int)*valp;
  // <MASK>
  *valp = (int)next_n;
  return p;
}