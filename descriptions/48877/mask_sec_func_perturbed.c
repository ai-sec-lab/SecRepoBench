static mrb_int
cmpnum(mrb_state *mrb, mrb_value v1, mrb_value value)
{
#ifdef // <MASK>

#ifdef MRB_NO_FLOAT
  mrb_int x, y;
#else
  mrb_float x, y;
#endif

#ifdef MRB_NO_FLOAT
  x = mrb_integer(v1);
#else
  x = mrb_as_float(mrb, v1);
#endif
  switch (mrb_type(value)) {
  case MRB_TT_INTEGER:
#ifdef MRB_NO_FLOAT
    y = mrb_integer(value);
#else
    y = (mrb_float)mrb_integer(value);
#endif
    break;
#ifndef MRB_NO_FLOAT
  case MRB_TT_FLOAT:
    y = mrb_float(value);
    break;
#ifdef MRB_USE_RATIONAL
  case MRB_TT_RATIONAL:
    y = mrb_as_float(mrb, value);
    break;
#endif
#endif
  default:
    return -2;
  }
  if (x > y)
    return 1;
  else {
    if (x < y)
      return -1;
    return 0;
  }
}