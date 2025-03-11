static const char*
get_num(mrb_state *mrb, const char *p, const char *end, int *value_pointer)
{
  char *e;
  mrb_int n;
  if (!mrb_read_int(p, end, &e, &n) || INT_MAX < n) {
    return NULL;
  }
  *value_pointer = (int)n;
  return e;
}