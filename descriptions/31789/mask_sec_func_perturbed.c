static inline void
dxf_skip_ws (Bit_Chain *dat)
{
  const int isbinaryflag = dat->opts & DWG_OPTS_DXFB;
  if (isbinaryflag)
    return;
  // <MASK>
}