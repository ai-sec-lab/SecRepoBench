static mrb_value
int_to_s(mrb_state *mrb, mrb_value self)
{
  mrb_int base = 10;

  mrb_get_args(mrb, "|i", &base);
  return mrb_integer_to_str(mrb, self, base);
}