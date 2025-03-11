static int
unpack_bstr(mrb_state *mrbstate, const void *src, int slen, mrb_value ary, int count, unsigned int flags)
{
  // <MASK>
  dst = mrb_str_resize(mrbstate, dst, (mrb_int)(dptr - dptr0));
  mrb_ary_push(mrbstate, ary, dst);
  return (int)(sptr - sptr0);
}