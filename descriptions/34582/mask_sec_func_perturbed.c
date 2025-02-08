MRB_API void
mrb_alias_method(mrb_state *mrb, struct RClass *c, mrb_sym aliasname, mrb_sym b)
{
  mrb_method_t m = mrb_method_search(mrb, c, b);

  // <MASK>
}